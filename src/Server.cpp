////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2010 - 2012, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>

#include <Server.h>
#include <Socket.h>
#include <Timer.h>

////////////////////////////////////////////////////////////////////////////////
Server::Server ()
  : _log (NULL)
  , _log_clients (false)
  , _client_address ("")
  , _client_port (0)
  , _port ("12345")
  , _pool_size (4)
  , _queue_size (10)
  , _daemon (false)
  , _pid_file ("")
  , _request_count (0)
  , _limit (0)        // Unlimited
  , _cert_file ()
  , _key_file ()
{
}

////////////////////////////////////////////////////////////////////////////////
Server::~Server ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Server::setPort (const std::string& port)
{
  if (_log) _log->format ("Using port %s", port.c_str ());
  _port = port;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setQueueSize (int size)
{
  if (_log) _log->format ("Queue size %d requests", size);
  _queue_size = size;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setPoolSize (int size)
{
  if (_log) _log->format ("Thread Pool size %d", size);
  _pool_size = size;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setDaemon ()
{
  if (_log) _log->write ("Will run as daemon");
  _daemon = true;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setPidFile (const std::string& file)
{
  if (_log) _log->write ("PID file " + file);
  assert (file.length () > 0);
  _pid_file = file;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setLimit (int max)
{
  if (_log) _log->format ("Request size limit %d bytes", max);
  assert (max >= 0);
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setCertFile (const std::string& file)
{
  if (_log) _log->format ("Certificate %s", file.c_str ());
  _cert_file = file;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setKeyFile (const std::string& file)
{
  if (_log) _log->format ("Private Key %s", file.c_str ());
  _key_file = file;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setLogClients (bool value)
{
  if (_log) _log->format ("IP logging %s", (value ? "on" : "off"));
  _log_clients = value;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setLog (Log* l)
{
  _log = l;
}

////////////////////////////////////////////////////////////////////////////////
void Server::beginServer ()
{
  if (_log) _log->write ("Server starting");

  if (_daemon)
  {
    daemonize ();  // Only the child returns.
    writePidFile ();
  }

  Socket s;
  s.bind (_port);
  s.listen (_queue_size);

  _request_count = 0;
  while (1)
  {
    try
    {
      int client = s.accept ();

      // Get client address and port, for logging.
      if (_log_clients)
      {
        struct sockaddr_storage addr;
        socklen_t len = sizeof (addr);
        char ipstr[INET6_ADDRSTRLEN];

        _client_address = "-";
        if (!getpeername (client, (struct sockaddr*) &addr, &len))
        {
          // deal with both IPv4 and IPv6:
          if (addr.ss_family == AF_INET)
          {
            struct sockaddr_in *s = (struct sockaddr_in *) &addr;
            _client_port = ntohs (s->sin_port);
            inet_ntop (AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
          }
          // AF_INET6
          else
          {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addr;
            _client_port = ntohs (s->sin6_port);
            inet_ntop (AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
          }

          _client_address = ipstr;
        }
      }

      // Metrics.
      HighResTimer timer;
      timer.start ();

      std::string input;
      Socket in (client);
      in.read (input);

      // Handle the request.
      ++_request_count;

      // Call the derived class handler.
      std::string output;
      handler (input, output);
      if (output.length ())
        in.write (output);

      if (_log)
      {
        timer.stop ();
        _log->format ("[%d] Serviced in %.6fs", _request_count, timer.total ());
      }
    }

    catch (std::string& e) { if (_log) _log->write (std::string ("Error: ") + e); }
    catch (char* e)        { if (_log) _log->write (std::string ("Error: ") + e); }
    catch (...)            { if (_log) _log->write ("Error: Unknown exception"); }
  }
}

////////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_OPENSSL
void Server::beginSSLServer ()
{
  if (_log) _log->write ("SSL Server Starting");

  if (_daemon)
  {
    daemonize ();  // Only the child returns.
    writePidFile ();
  }

  _request_count = 0;
  try
  {
    BIO* bio_ssl;
    BIO* bio_incoming;
    BIO* bio_connection;

    SSL_library_init ();
    SSL_load_error_strings ();

    // Select protocol version in the following order:
    // - use pure TLSv1 if it is the only protocol version available
    // - use SSLv3 support optionally in an SSLv2 handshake
    //   (for maximum compatibility) (if possible)
    // - use pure SSLv3
    // - use pure SSLv2
    SSL_METHOD* method = NULL;
#if defined(NO_SSL2) && defined(NO_SSL3) && !defined(NO_TLS1)
    method = TLSv1_server_method ();
    if (_log) _log->write ("TLSv1 connection type");
#elif (!defined(NO_SSL2) || defined(NO_SSL2IMPL)) && !defined(NO_SSL3)
    method = SSLv23_server_method ();
    if (_log) _log->write ("SSLv23 connection type");
#elif !defined(NO_SSL3)
    method = SSLv3_server_method ();
    if (_log) _log->write ("SSLv3 connection type");
#elif !defined(NO_SSL2)
    method = SSLv2_server_method ();
    if (_log) _log->write ("SSLv2 connection type");
#else
    throw std::string ("ERROR: No suitable SSL protocol type available");
#endif

    SSL_CTX* ctx = SSL_CTX_new (method);

    // Enable all vendor bug compatibility options.
    SSL_CTX_set_options (ctx, SSL_OP_ALL);

    if (SSL_CTX_use_certificate_file (ctx, _cert_file.c_str (), SSL_FILETYPE_PEM) < 1)
      throw std::string ("ERROR: While loading public cert '") + _cert_file + "'";

    if (SSL_CTX_use_PrivateKey_file (ctx, _key_file.c_str (), SSL_FILETYPE_PEM) < 1)
      throw std::string ("ERROR: While loading private key '") + _key_file + "'";

    // Check that the certificate and private key match.
    if (! SSL_CTX_check_private_key (ctx))
      throw std::string ("ERROR: No matching cert '") + _cert_file + "' to private key '" + _key_file + "'";

    bio_ssl = BIO_new_ssl (ctx, 0);
    if (bio_ssl == NULL)
      throw std::string ("ERROR: Cannot create server socket");

    SSL* ssl;
    BIO_get_ssl (bio_ssl, &ssl);
    SSL_set_mode (ssl, SSL_MODE_AUTO_RETRY);

    bio_incoming = BIO_new_accept (_port.c_str ());
    BIO_set_accept_bios (bio_incoming, bio_ssl);

    // First call sets up socket.
    if (BIO_do_accept (bio_incoming) <= 0)
      throw std::string ("ERROR: Cannot bind server socket");

    for (;;)
    {
      if (BIO_do_accept (bio_incoming) <= 0)
        throw std::string ("ERROR: Cannot accept connection");

      bio_connection = BIO_pop (bio_incoming);

      // Switch on the non-blocking I/O flag for the connection BIO bio_connection
      // TODO This call is misbehaving and not returning 1, as promised.
      /*if (*/BIO_set_nbio (bio_connection, 1)/* != 1)
        throw std::string ("ERROR: Unable to set non-blocking mode")*/;

      // Preempt an in-session handshake request.
      if (BIO_do_handshake (bio_connection) <= 0)
        throw std::string ("ERROR: Problem during handshake");

      // Get client address and port, for logging.
      if (_log_clients)
      {
        struct sockaddr_storage addr;
        socklen_t len = sizeof (addr);
        char ipstr[INET6_ADDRSTRLEN];

        int client_fd = BIO_get_fd (bio_connection, NULL);

        _client_address = "";
        if (!getpeername (client_fd, (struct sockaddr*) &addr, &len))
        {
          // deal with both IPv4 and IPv6:
          if (addr.ss_family == AF_INET)
          {
            struct sockaddr_in *s = (struct sockaddr_in *) &addr;
            _client_port = ntohs (s->sin_port);
            inet_ntop (AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
          }
          // AF_INET6
          else
          {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addr;
            _client_port = ntohs (s->sin6_port);
            inet_ntop (AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
          }

          _client_address = ipstr;
        }
      }

      // Metrics.
      HighResTimer timer;
      timer.start ();

      // Handle the request.
      ++_request_count;

      // Read, process request, write response.
      std::string input = read (bio_connection);
      std::string output;
      handler (input, output);
      write (bio_connection, output);

      BIO_free_all (bio_connection);
      ERR_remove_state (0);

      if (_log)
      {
        timer.stop ();
        _log->format ("[%d] Serviced in %.6fs", _request_count, timer.total ());
      }
    }

    BIO_free_all (bio_incoming);
  }

  catch (std::string& e) { if (_log) _log->write (std::string ("Error: ") + e); }
  catch (char* e)        { if (_log) _log->write (std::string ("Error: ") + e); }
  catch (...)            { if (_log) _log->write ("Error: Unknown exception"); }
}

////////////////////////////////////////////////////////////////////////////////
std::string Server::read (BIO* bio)
{
  std::string input;
  int error;
  char buf[1024];

  do
  {
    error = BIO_read (bio, buf, sizeof (buf) - 1);
    if (error > 0)
    {
      buf[error] = 0;
      input += buf;
    }
    else
    {
      if (! BIO_should_retry (bio))
        return input;

      error = 0;
    }
  }
  while (error == 0 || error == sizeof (buf) - 1);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
bool Server::write (BIO* bio, const std::string& output)
{
  int error = 0;
  for (int bytes = 0; bytes < output.size (); bytes += error)
  {
    error = BIO_write (bio, output.c_str () + bytes, output.size () - bytes);
    if (error <= 0)
    {
      if (! BIO_should_retry (bio))
        return false;

      error = 0;
    }
  }

  return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// TODO To provide these data, a request count, a start time, and a cumulative
//      utilization time must be tracked.
void Server::stats (int& requests, time_t& uptime, double& utilization)
{
  requests = _request_count;
  uptime = 0;
  utilization = 0.0;
}

////////////////////////////////////////////////////////////////////////////////
void Server::daemonize ()
{
  if (_log) _log->write ("Daemonizing");

/* TODO Load RUN_AS_USER from config.

  // If run as root, switch to preferred user.
  if (getuid () == 0 || geteuid () == 0 )
  {
    struct passwd *pw = getpwnam (RUN_AS_USER);
    if (pw)
    {
      if (_log) _log->write ("setting user to " RUN_AS_USER);
      setuid (pw->pw_uid);
    }
  }
*/

  // Fork off the parent process
  pid_t pid = fork ();
  if (pid < 0)
  {
    exit (EXIT_FAILURE);
  }

  // If we got a good PID, then we can exit the parent process.
  if (pid > 0)
  {
    exit (EXIT_SUCCESS);
  }

  // Change the file mode mask
  umask (0);

  // Create a new SID for the child process
  pid_t sid = setsid ();
  if (sid < 0)
  {
    if (_log) _log->write ("setsid failed");
    exit (EXIT_FAILURE);
  }

  // Change the current working directory
  // Why is this important?  To ensure that program is independent of $CWD?
  if ((chdir ("/")) < 0)
  {
    if (_log) _log->write ("chdir failed");
    exit (EXIT_FAILURE);
  }

  // Redirect standard files to /dev/null.
  freopen ("/dev/null", "r", stdin);
  freopen ("/dev/null", "w", stdout);
  freopen ("/dev/null", "w", stderr);

  if (_log) _log->write ("Daemonized");
}

////////////////////////////////////////////////////////////////////////////////
void Server::writePidFile ()
{
  pid_t pid = getpid ();

  FILE* output = fopen (_pid_file.c_str (), "w");
  if (output)
  {
    fprintf (output, "%d", pid);
    fclose (output);
  }
  else
    if (_log) _log->write ("Error: could not write PID to '" + _pid_file + "'.");
}

////////////////////////////////////////////////////////////////////////////////
void Server::removePidFile ()
{
  assert (_pid_file.length () > 0);
  unlink (_pid_file.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
