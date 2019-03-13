////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2018, Göteborg Bit Factory.
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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <Server.h>
#include <TLSServer.h>
#include <Timer.h>
#include <format.h>

// Indicates that certain signals were caught.
bool _sighup  = false;
bool _sigusr1 = false;
bool _sigusr2 = false;

////////////////////////////////////////////////////////////////////////////////
static void signal_handler (int s)
{
  switch (s)
  {
  case SIGHUP:  _sighup  = true; break;  // Graceful stop
  case SIGUSR1: _sigusr1 = true; break;  // Config reload
  case SIGUSR2: _sigusr2 = true; break;
  }
}

////////////////////////////////////////////////////////////////////////////////
Server::Server ()
{
}

////////////////////////////////////////////////////////////////////////////////
Server::~Server ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Server::setPort (const std::string& port)
{
  if (_log) _log->write (format ("Using port {1}", port));
  _port = port;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setHost (const std::string& host)
{
  if (_log) _log->write (format ("Using address {1}", host));
  _host = host;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setFamily (const std::string& family)
{
  if (_log) _log->write (format ("Using family {1}", family));
  _family = family;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setQueueSize (int size)
{
  if (_log) _log->write (format ("Queue size {1} requests", size));
  _queue_size = size;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setPoolSize (int size)
{
  if (_log) _log->write (format ("Thread Pool size {1}", size));
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
  if (_log) _log->write (format ("Request size limit {1} bytes", max));
  assert (max >= 0);
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setCAFile (const std::string& file)
{
  File cert (file);
  if (! cert.exists ())
    return;
  if (_log) _log->write (format ("CA          {1}", file));
  _ca_file = file;
  if (! cert.readable ())
    throw format ("CA Certificate not readable: '{1}'", file);
}

////////////////////////////////////////////////////////////////////////////////
void Server::setCertFile (const std::string& file)
{
  if (_log) _log->write (format ("Certificate {1}", file));
  _cert_file = file;
  File cert (file);
  if (! cert.readable ())
    throw format ("Server Certificate not readable: '{1}'", file);
}

////////////////////////////////////////////////////////////////////////////////
void Server::setKeyFile (const std::string& file)
{
  if (_log) _log->write (format ("Private Key {1}", file));
  _key_file = file;
  File key (file);
  if (! key.readable ())
    throw format ("Server key not readable: '{1}'", file);
}

////////////////////////////////////////////////////////////////////////////////
void Server::setCRLFile (const std::string& file)
{
  File crl (file);
  if (! crl.exists ())
    return;
  if (_log) _log->write (format ("CRL         {1}", file));
  _crl_file = file;
  if (! crl.readable ())
    throw format ("CRL Certificate not readable: '{1}'", file);
}

////////////////////////////////////////////////////////////////////////////////
void Server::setLogClients (bool value)
{
  if (_log) _log->write (format ("IP logging {1}", (value ? "on" : "off")));
  _log_clients = value;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setLog (Log* l)
{
  _log = l;
}

////////////////////////////////////////////////////////////////////////////////
void Server::setConfig (Config* c)
{
  _config = c;
}

////////////////////////////////////////////////////////////////////////////////
void Server::beginServer ()
{
  if (_config)
  {
    setCAFile (_config->get ("ca.cert"));
    setCertFile (_config->get ("server.cert"));
    setKeyFile (_config->get ("server.key"));
    setCRLFile (_config->get ("server.crl"));
  }

  if (_log) _log->write ("Server starting");

  if (_daemon)
  {
    daemonize ();  // Only the child returns.
    writePidFile ();
  }

  if (signal (SIGHUP,  signal_handler) == SIG_ERR) // Graceful stop
    throw std::string ("Failed to register handler for SIGHUP... Exiting.");
  if (signal (SIGUSR1, signal_handler) == SIG_ERR) // Config reload
    throw std::string ("Failed to register handler for SIGUSR1... Exiting.");
  if (signal (SIGUSR2, signal_handler) == SIG_ERR)
    throw std::string ("Failed to register handler for SIGUSR2... Exiting.");

  TLSServer server;
  if (_config)
  {
    server.debug (_config->getInteger ("debug.tls"));

    std::string ciphers = _config->get ("ciphers");
    if (ciphers != "")
    {
      server.ciphers (ciphers);
      if (_log) _log->write (format ("Using ciphers: {1}", ciphers));
    }

    std::string trust = _config->get ("trust");
    if (trust == "allow all")
      server.trust (TLSServer::allow_all);
    else if (trust == "strict")
      server.trust (TLSServer::strict);
    else if (_log)
      _log->write (format ("Invalid 'trust' setting value of '{1}'", trust));

    int dh_bits = _config->getInteger ("dh_bits");
    if (dh_bits < 0)
    {
      if (_log) _log->write (format ("Invalid dh_bits value, using defaults: {1}", dh_bits));
      dh_bits = 0;
    }

    server.dh_bits (dh_bits);
    if (_log) _log->write (format ("Using dh_bits: {1}", dh_bits));
  }

  server.init (_ca_file,        // CA
               _crl_file,       // CRL
               _cert_file,      // Cert
               _key_file);      // Key
  server.queue (_queue_size);
  server.bind (_host, _port, _family);
  server.listen ();

  if (_log) _log->write ("Server ready");

  _request_count = 0;
  while (1)
  {
    try
    {
      TLSTransaction tx;
      tx.trust (server.trust ());
      server.accept (tx);

      if (_sighup)
        throw "SIGHUP shutdown.";

      // Get client address and port, for logging.
      if (_log_clients)
        tx.getClient (_client_address, _client_port);

      // Metrics.
      Timer timer;
      timer.start ();

      std::string input;
      tx.recv (input);

      // Handle the request.
      ++_request_count;

      // Call the derived class handler.
      std::string output;
      handler (input, output);
      if (output.length ())
        tx.send (output);

      if (_log)
      {
        timer.stop ();
        _log->write (format ("[{1}] Serviced in {2}s", _request_count, (timer.total_us () / 1e6)));
      }
    }

    catch (std::string& e) { if (_log) _log->write (std::string ("Error: ") + e); }
    catch (char* e)        { if (_log) _log->write (std::string ("Error: ") + e); }
    catch (...)            { if (_log) _log->write ("Error: Unknown exception"); }
  }
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
