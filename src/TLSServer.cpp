////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2013, Paul Beckingham, Federico Hernandez.
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

#ifdef HAVE_LIBGNUTLS

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TLSServer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef OPENBSD
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <sys/types.h>
#include <netdb.h>

#define DH_BITS 1024
#define MAX_BUF 16384

static int verify_certificate_callback (gnutls_session_t);

static bool trust_override = false;

////////////////////////////////////////////////////////////////////////////////
static void gnutls_log_function (int level, const char* message)
{
  std::cout << "s: " << level << " " << message;
}

////////////////////////////////////////////////////////////////////////////////
static int verify_certificate_callback (gnutls_session_t session)
{
  if (trust_override)
    return 0;

  // This verification function uses the trusted CAs in the credentials
  // structure. So you must have installed one or more CA certificates.
  unsigned int status = 0;
#if GNUTLS_VERSION_NUMBER >= 0x030104
  int ret = gnutls_certificate_verify_peers3 (session, NULL, &status);
#else
  int ret = gnutls_certificate_verify_peers2 (session, &status);
#endif
  if (ret < 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

#if GNUTLS_VERSION_NUMBER >= 0x030105
  gnutls_certificate_type_t type = gnutls_certificate_type_get (session);
  gnutls_datum_t out;
  ret = gnutls_certificate_verification_status_print (status, type, &out, 0);
  if (ret < 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  gnutls_free (out.data);
#endif

  if (status != 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  // Continue handshake.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
TLSServer::TLSServer ()
: _crl ("")
, _cert ("")
, _key ("")
, _socket (0)
, _queue (5)
, _debug (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TLSServer::~TLSServer ()
{
  gnutls_certificate_free_credentials (_credentials);
  gnutls_priority_deinit (_priorities);
  gnutls_global_deinit ();

  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::queue (int depth)
{
  _queue = depth;
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TLSServer::debug (int level)
{
  if (level)
    _debug = true;

  gnutls_global_set_log_function (gnutls_log_function);
  gnutls_global_set_log_level (level);
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::trust (bool value)
{
  trust_override = value;
  if (_debug)
  {
    if (trust_override)
      std::cout << "s: INFO Client certificate trusted automatically.\n";
    else
      std::cout << "s: INFO Client certificate trust verified.\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::init (
  const std::string& ca,
  const std::string& crl,
  const std::string& cert,
  const std::string& key)
{
  _ca   = ca;
  _crl = crl;
  _cert = cert;
  _key = key;

  gnutls_global_init ();
  gnutls_certificate_allocate_credentials (&_credentials);
  if (_ca != "" &&
      gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM) < 0)
    throw std::string ("Missing CA file.");

  if ( _crl != "" &&
      gnutls_certificate_set_x509_crl_file (_credentials, _crl.c_str (), GNUTLS_X509_FMT_PEM) < 0)
    throw std::string ("Missing CRL file.");

  if (_cert != "" &&
      _key != "" &&
      gnutls_certificate_set_x509_key_file (_credentials, _cert.c_str (), _key.c_str (), GNUTLS_X509_FMT_PEM) < 0)
    throw std::string ("Missing CERT file.");

#if GNUTLS_VERSION_NUMBER >= 0x020b00
#if GNUTLS_VERSION_NUMBER >= 0x03000d
  unsigned int bits = gnutls_sec_param_to_pk_bits (GNUTLS_PK_DH, GNUTLS_SEC_PARAM_LEGACY);
#else
  unsigned int bits = gnutls_sec_param_to_pk_bits (GNUTLS_PK_DH, GNUTLS_SEC_PARAM_NORMAL);
#endif
#else
  unsigned int bits = DH_BITS;
#endif
  gnutls_dh_params_init (&_params);
  gnutls_dh_params_generate2 (_params, bits);

  gnutls_priority_init (&_priorities, "PERFORMANCE:%SERVER_PRECEDENCE", NULL);
  gnutls_certificate_set_dh_params (_credentials, _params);

#if GNUTLS_VERSION_NUMBER >= 0x02090a
  gnutls_certificate_set_verify_function (_credentials, verify_certificate_callback);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::bind (const std::string& port)
{
  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints = {0};
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  if (::getaddrinfo (NULL, port.c_str (), &hints, &res) != 0)
    throw std::string (::gai_strerror (errno));

  if ((_socket = ::socket (res->ai_family,
                           res->ai_socktype,
                           res->ai_protocol)) == -1)
    throw std::string ("Can not bind to port ") + port;

  // When a socket is closed, it remains unavailable for a while (netstat -an).
  // Setting SO_REUSEADDR allows this program to assume control of a closed, but
  // unavailable socket.
  int on = 1;
  if (::setsockopt (_socket,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const void*) &on,
                    sizeof (on)) == -1)
    throw std::string (::strerror (errno));

  if (::bind (_socket, res->ai_addr, res->ai_addrlen) == -1)
    throw std::string (::strerror (errno));
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::listen ()
{
  if (::listen (_socket, _queue) < 0)
    throw std::string (::strerror (errno));

  if (_debug)
    std::cout << "s: INFO Server listening.\n";
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::accept (TLSTransaction& tx)
{
  if (_debug)
    tx.debug ();

  tx.init (*this);
}

////////////////////////////////////////////////////////////////////////////////
TLSTransaction::TLSTransaction ()
: _socket (0)
, _limit (0)
, _debug (false)
, _address ("")
, _port (0)
{
}

////////////////////////////////////////////////////////////////////////////////
TLSTransaction::~TLSTransaction ()
{
  if (_socket)
  {
    close (_socket);
    _socket = 0;
  }

  gnutls_deinit (_session);
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::init (TLSServer& server)
{
  gnutls_init (&_session, GNUTLS_SERVER);
  gnutls_priority_set (_session, server._priorities);
  gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, server._credentials);

  // Require client certificate.
  gnutls_certificate_server_set_request (_session, GNUTLS_CERT_REQUIRE);

/*
  // Set maximum compatibility mode. This is only suggested on public
  // webservers that need to trade security for compatibility
  gnutls_session_enable_compatibility_mode (_session);
*/

  struct sockaddr_in sa_cli = {0};
  socklen_t client_len = sizeof sa_cli;
  do
  {
    _socket = accept (server._socket, (struct sockaddr *) &sa_cli, &client_len);
  }
  while (errno == EINTR);

  if (_socket < 0)
    throw std::string (::strerror (errno));

  // Obtain client info.
  char topbuf[512];
  _address = inet_ntop (AF_INET, &sa_cli.sin_addr, topbuf, sizeof (topbuf));
  _port    = ntohs (sa_cli.sin_port);
  if (_debug)
    std::cout << "s: INFO connection from "
              << _address
              << " port "
              << _port
              << "\n";

#if GNUTLS_VERSION_NUMBER >= 0x030109
  gnutls_transport_set_int (_session, _socket);
#else
  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) (long) _socket);
#endif

  // Key exchange.
  int ret;
  do
  {
    ret = gnutls_handshake (_session);
  }
  while (ret < 0 && gnutls_error_is_fatal (ret) == 0);

  if (ret < 0)
    throw std::string ("Handshake has failed (") + gnutls_strerror (ret) + ")";

  if (_debug)
    std::cout << "s: INFO Handshake was completed\n";
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::bye ()
{
  gnutls_bye (_session, GNUTLS_SHUT_RDWR);
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TLSTransaction::debug ()
{
  _debug = true;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::limit (int max)
{
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::send (const std::string& data)
{
  std::string packet = "XXXX" + data;

  // Encode the length.
  unsigned long l = packet.length ();
  packet[0] = l >>24;
  packet[1] = l >>16;
  packet[2] = l >>8;
  packet[3] = l;

  unsigned int total = 0;
  unsigned int remaining = packet.length ();

  while (total < packet.length ())
  {
    int status;
    do
    {
      status = gnutls_record_send (_session, packet.c_str () + total, remaining);
    }
    while (errno == GNUTLS_E_INTERRUPTED ||
           errno == GNUTLS_E_AGAIN);

    if (status == -1)
      break;

    total     += (unsigned int) status;
    remaining -= (unsigned int) status;
  }

  if (_debug)
    std::cout << "s: INFO Sending 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::recv (std::string& data)
{
  data = "";          // No appending of data.
  int received = 0;

  // Get the encoded length.
  unsigned char header[4] = {0};
  do
  {
    received = gnutls_record_recv (_session, header, 4);
  }
  while (received > 0 &&
         (errno == GNUTLS_E_INTERRUPTED ||
          errno == GNUTLS_E_AGAIN));

  int total = received;

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];
  if (_debug)
    std::cout << "s: INFO expecting " << expected << " bytes.\n";

  // TODO This would be a good place to assert 'expected < _limit'.

  // Arbitrary buffer size.
  char buffer[MAX_BUF];

  // Keep reading until no more data.  Concatenate chunks of data if a) the
  // read was interrupted by a signal, and b) if there is more data than
  // fits in the buffer.
  do
  {
    do
    {
      received = gnutls_record_recv (_session, buffer, MAX_BUF - 1);
    }
    while (received > 0 &&
           (errno == GNUTLS_E_INTERRUPTED ||
            errno == GNUTLS_E_AGAIN));

    // Other end closed the connection.
    if (received == 0)
    {
      if (_debug)
        std::cout << "s: INFO Peer has closed the TLS connection\n";
      break;
    }

    // Something happened.
    if (received < 0)
      throw std::string (gnutls_strerror (received));

    buffer [received] = '\0';
    data += buffer;
    total += received;

    // Stop at defined limit.
    if (_limit && total > _limit)
      break;
  }
  while (received > 0 && total < (int) expected);

  if (_debug)
    std::cout << "s: INFO Receiving 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::getClient (std::string& address, int& port)
{
  address = _address;
  port = _port;
}

////////////////////////////////////////////////////////////////////////////////
#endif
