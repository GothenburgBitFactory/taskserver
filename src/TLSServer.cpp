////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Göteborg Bit Factory.
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
#include <stdint.h>
#include <string.h>
#include <TLSServer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if (defined OPENBSD || defined SOLARIS)
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <sys/types.h>
#include <netdb.h>
#include <gnutls/x509.h>
#include <format.h>

#define DH_BITS 2048
#define MAX_BUF 16384

#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
static int verify_certificate_callback (gnutls_session_t);
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
static void gnutls_log_function (int level, const char* message)
{
  std::cout << "s: " << level << " " << message;
}

////////////////////////////////////////////////////////////////////////////////
#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
static int verify_certificate_callback (gnutls_session_t session)
{
  const TLSTransaction* tx = (TLSTransaction*) gnutls_session_get_ptr (session); // All
  return tx->verify_certificate ();
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
TLSServer::TLSServer ()
{
  // Set up the default.
  dh_bits (0);
}

////////////////////////////////////////////////////////////////////////////////
TLSServer::~TLSServer ()
{
  if (_credentials)
    gnutls_certificate_free_credentials (_credentials);

  if(_priorities && _priorities_init)
    gnutls_priority_deinit (_priorities);

#if GNUTLS_VERSION_NUMBER < 0x030300
  // Not needed after v3.3.0, handled automatically by library.
  gnutls_global_deinit ();
#endif

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

  gnutls_global_set_log_function (gnutls_log_function); // All
  gnutls_global_set_log_level (level); // All
}

////////////////////////////////////////////////////////////////////////////////
enum TLSServer::trust_level TLSServer::trust () const
{
  return _trust;
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::trust (const enum trust_level value)
{
  _trust = value;
  if (_debug)
  {
    if (_trust == allow_all)
      std::cout << "s: INFO Client certificate will be trusted automatically.\n";
    else
      std::cout << "s: INFO Client certificate will be verified.\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::ciphers (const std::string& cipher_list)
{
  _ciphers = cipher_list;
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::dh_bits (unsigned int dh_bits)
{
  if (!dh_bits)
#if GNUTLS_VERSION_NUMBER >= 0x020b00
#if GNUTLS_VERSION_NUMBER >= 0x03000d
    dh_bits = gnutls_sec_param_to_pk_bits (GNUTLS_PK_DH, GNUTLS_SEC_PARAM_LEGACY); // 3.0.13 for GNUTLS_SEC_PARAM_LEGACY
#else
    dh_bits = gnutls_sec_param_to_pk_bits (GNUTLS_PK_DH, GNUTLS_SEC_PARAM_NORMAL); // 2.12.0
#endif
#else
    dh_bits = DH_BITS;
#endif
  _dh_bits = dh_bits;
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::init (
  const std::string& ca,
  const std::string& crl,
  const std::string& cert,
  const std::string& key)
{
  _ca   = ca;
  _crl  = crl;
  _cert = cert;
  _key  = key;

  int ret;
#if GNUTLS_VERSION_NUMBER < 0x030300
  ret = gnutls_global_init (); // All
  if (ret < 0)
    throw format ("TLS init error. {1}", gnutls_strerror (ret)); // All
#endif

  ret = gnutls_certificate_allocate_credentials (&_credentials); // All
  if (ret < 0)
    throw format ("TLS allocation error. {1}", gnutls_strerror (ret)); // All

#if GNUTLS_VERSION_NUMBER >= 0x030014
  // Automatic loading of system installed CA certificates.
  ret = gnutls_certificate_set_x509_system_trust (_credentials); // 3.0.20
  if (ret < 0)
    throw format ("Bad System Trust. {1}", gnutls_strerror (ret)); // All
#endif

  if (_ca != "" &&
      (ret = gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM)) < 0) // All
    throw format ("Bad CA file. {1}", gnutls_strerror (ret)); // All

  if ( _crl != "" &&
      (ret = gnutls_certificate_set_x509_crl_file (_credentials, _crl.c_str (), GNUTLS_X509_FMT_PEM)) < 0) // All
    throw format ("Bad CRL file. {1}", gnutls_strerror (ret)); // All

  // TODO This may need 0x030111 protection.
  if (_cert != "" &&
      _key != "" &&
      (ret = gnutls_certificate_set_x509_key_file (_credentials, _cert.c_str (), _key.c_str (), GNUTLS_X509_FMT_PEM)) < 0) // 3.1.11
    throw format ("Bad CERT file. {1}", gnutls_strerror (ret)); // All

  if (_ciphers == "")
    _ciphers =
      "%SERVER_PRECEDENCE"          // use the server's precedences for algorithms
      ":NORMAL"                     // the normal suite
      ":-VERS-SSL3.0:-VERS-TLS1.0"  // SSLv3 and TLSv1.0 are vulnerable to POODLE
      ":-3DES-CBC"                  // 3DES is broken with CBC
      ":-ARCFOUR-128:-ARCFOUR-40"   // RC4 is broken
      ":-MD5";                      // MD5 is not good enough anymore
  ret = gnutls_priority_init (&_priorities, _ciphers.c_str (), NULL); // All
  if ( ret < 0 )
      throw format("couldn't initialize priorities: {1}", gnutls_strerror(ret));
  _priorities_init = true;

#if GNUTLS_VERSION_NUMBER >= 0x030506
  gnutls_certificate_set_known_dh_params (_credentials, GNUTLS_SEC_PARAM_HIGH); // 3.5.6
#else
  gnutls_dh_params_t params;
  ret = gnutls_dh_params_init (&params); // All
  if (ret < 0)
    throw format ("couldn't initialize DH parameters: {1}", gnutls_strerror (ret));
  ret = gnutls_dh_params_generate2 (params, _dh_bits); // All
  if (ret < 0)
    throw format ("couldn't generate DH parameters: {1}", gnutls_strerror (ret));
  gnutls_certificate_set_dh_params (_credentials, params); // All
#endif

#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
  // The automatic verification for the client certificate with
  // gnutls_certificate_set_verify_function only works with gnutls
  // >=2.10.0. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  gnutls_certificate_set_verify_function (_credentials, verify_certificate_callback); // 2.10.0
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////
void TLSServer::bind (
  const std::string& host,
  const std::string& port,
  const std::string& family)
{
  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints {};
  hints.ai_family   = (family == "IPv6" ? AF_INET6 :
                       family == "IPv4" ? AF_INET  :
                                          AF_UNSPEC);
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  int ret = ::getaddrinfo (host.c_str (), port.c_str (), &hints, &res);
  if (ret != 0)
    throw std::string (::gai_strerror (ret));

  for (struct addrinfo* p = res; p != NULL; p = p->ai_next)
  {
    // IPv4
    void *addr;
    int ipver;
    if (p->ai_family == AF_INET)
    {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = 4;
    }
    // IPv6
    else
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = 6;
    }

    // Convert the IP to a string and print it:
    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop (p->ai_family, addr, ipstr, sizeof ipstr);
    if (_debug)
      std::cout << "s: INFO IPv" << ipver << ": " << ipstr << '\n';
  }

  if ((_socket = ::socket (res->ai_family,
                           res->ai_socktype,
                           res->ai_protocol)) == -1)
    throw std::string ("Can not bind to  ") + host + port;

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

  // Also listen to IPv4 with IPv6 in dual-stack situations
  if (res->ai_family == AF_INET6)
  {
    int off = 0;
    if (::setsockopt (_socket,
                      IPPROTO_IPV6,
                      IPV6_V6ONLY,
                      (const void*) &off,
                      sizeof (off)) == -1)
      if (_debug)
        std::cout << "s: Unable to use IPv6 dual stack\n";
  }

  if (::bind (_socket, res->ai_addr, res->ai_addrlen) == -1)
  {
    // TODO This is research to determine whether this is the right location to
    //      inject a helpful log message, such as
    //
    //      "Check to see if the server is already running."
    std::cout << "### bind failed\n";
    throw std::string (::strerror (errno));
  }
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
TLSTransaction::~TLSTransaction ()
{
  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
    _socket = 0;
  }

  gnutls_deinit (_session); // All
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::init (TLSServer& server)
{
  int ret = gnutls_init (&_session, GNUTLS_SERVER); // All
  if (ret < 0)
    throw format ("TLS server init error. {1}", gnutls_strerror (ret)); // All

  ret = gnutls_priority_set (_session, server._priorities); // All
  if (ret < 0)
    throw format ("Error initializing TLS. {1}", gnutls_strerror (ret)); // All

  // Apply the x509 credentials to the current session.
  ret = gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, server._credentials); // All
  if (ret < 0)
    throw format ("TLS credentials error. {1}", gnutls_strerror (ret)); // All

  // Store the TLSTransaction instance, so that the verification callback can
  // access it during the handshake below and call the verifcation method.
  gnutls_session_set_ptr (_session, (void*) this); // All

  // Require client certificate.
  gnutls_certificate_server_set_request (_session, GNUTLS_CERT_REQUIRE); // All

/*
  // Set maximum compatibility mode. This is only suggested on public
  // webservers that need to trade security for compatibility
  gnutls_session_enable_compatibility_mode (_session);
*/

  struct sockaddr_in sa_cli {};
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
              << '\n';

#if GNUTLS_VERSION_NUMBER >= 0x030109
  gnutls_transport_set_int (_session, _socket); // 3.1.9
#else
  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) (intptr_t) _socket); // All
#endif

  // Perform the TLS handshake
  do
  {
    ret = gnutls_handshake (_session); // All
  }
  while (ret < 0 && gnutls_error_is_fatal (ret) == 0); // All

  if (ret < 0)
  {
#if GNUTLS_VERSION_NUMBER >= 0x030406
    if (ret == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR)
    {
      auto type = gnutls_certificate_type_get (_session); // All
      auto status = gnutls_session_get_verify_cert_status (_session); // 3.4.6
      gnutls_datum_t out;
      gnutls_certificate_verification_status_print (status, type, &out, 0);  // 3.1.4
      gnutls_free (out.data); // All

      std::string error {(const char*) out.data};
      throw format ("Handshake failed. {1}", error);
    }
#else
    throw format ("Handshake failed. {1}", gnutls_strerror (ret)); // All
#endif
  }

#if GNUTLS_VERSION_NUMBER < 0x02090a
  // The automatic verification for the server certificate with
  // gnutls_certificate_set_verify_function does only work with gnutls
  // >=2.9.10. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  ret = verify_certificate ();
  if (ret < 0)
  {
    if (_debug)
      std::cout << "s: ERROR Certificate verification failed.\n";
    throw format ("Error Initializing TLS. {1}", gnutls_strerror (ret)); // All
  }
#endif

  if (_debug)
  {
#if GNUTLS_VERSION_NUMBER >= 0x03010a
    char* desc = gnutls_session_get_desc (_session); // 3.1.10
    std::cout << "s: INFO Handshake was completed: " << desc << '\n';
    gnutls_free (desc);
#else
    std::cout << "s: INFO Handshake was completed.\n";
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::bye ()
{
  gnutls_bye (_session, GNUTLS_SHUT_RDWR); // All
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TLSTransaction::debug ()
{
  _debug = true;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::trust (const enum TLSServer::trust_level value)
{
  _trust = value;
}

////////////////////////////////////////////////////////////////////////////////
void TLSTransaction::limit (int max)
{
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
int TLSTransaction::verify_certificate () const
{
  if (_trust == TLSServer::allow_all)
    return 0;

  if (_debug)
    std::cout << "s: INFO Verifying certificate.\n";

  // This verification function uses the trusted CAs in the credentials
  // structure. So you must have installed one or more CA certificates.
  unsigned int status = 0;
#if GNUTLS_VERSION_NUMBER >= 0x030104
  int ret = gnutls_certificate_verify_peers3 (_session, NULL, &status); // 3.1.4
  if (ret < 0)
  {
    if (_debug)
      std::cout << "s: ERROR Certificate verification peers3 failed. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  // status 16450 == 0100000001000010
  //   GNUTLS_CERT_INVALID             1<<1
  //   GNUTLS_CERT_SIGNER_NOT_FOUND    1<<6
  //   GNUTLS_CERT_UNEXPECTED_OWNER    1<<14  Hostname does not match

  if (_debug && status)
    std::cout << "s: ERROR Certificate status=" << status << '\n';
#else
  int ret = gnutls_certificate_verify_peers2 (_session, &status); // All
  if (ret < 0)
  {
    if (_debug)
      std::cout << "s: ERROR Certificate verification peers2 failed. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  if (_debug && status)
    std::cout << "s: ERROR Certificate status=" << status << '\n';

  if (status == 0)
  {
    if (gnutls_certificate_type_get (_session) == GNUTLS_CRT_X509) // All
    {
      const gnutls_datum* cert_list;
      unsigned int cert_list_size;
      gnutls_x509_crt cert;

      cert_list = gnutls_certificate_get_peers (_session, &cert_list_size); // All
      if (cert_list_size == 0)
      {
        if (_debug)
          std::cout << "s: ERROR Certificate get peers failed. " << gnutls_strerror (ret) << '\n'; // All
        return GNUTLS_E_CERTIFICATE_ERROR;
      }

      ret = gnutls_x509_crt_init (&cert); // All
      if (ret < 0)
      {
        if (_debug)
          std::cout << "s: ERROR x509 init failed. " << gnutls_strerror (ret) << '\n'; // All
        return GNUTLS_E_CERTIFICATE_ERROR;
      }

      ret = gnutls_x509_crt_import (cert, &cert_list[0], GNUTLS_X509_FMT_DER); // All
      if (ret < 0)
      {
        if (_debug)
          std::cout << "s: ERROR x509 cert import. " << gnutls_strerror (ret) << '\n'; // All
        gnutls_x509_crt_deinit(cert);
        status = GNUTLS_E_CERTIFICATE_ERROR;
      }
    }
    else
      return GNUTLS_E_CERTIFICATE_ERROR;
  }
#endif

#if GNUTLS_VERSION_NUMBER >= 0x030104
  gnutls_certificate_type_t type = gnutls_certificate_type_get (_session); // All
  gnutls_datum_t out;
  ret = gnutls_certificate_verification_status_print (status, type, &out, 0); // 3.1.4
  if (ret < 0)
  {
    if (_debug)
      std::cout << "s: ERROR certificate verification status. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  if (_debug)
    std::cout << "s: INFO " << out.data << '\n';
  gnutls_free (out.data);
#endif

  if (status != 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  // Continue handshake.
  return 0;
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
      status = gnutls_record_send (_session, packet.c_str () + total, remaining); // All
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
  unsigned char header[4] {};
  do
  {
    received = gnutls_record_recv (_session, header, 4); // All
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
      received = gnutls_record_recv (_session, buffer, MAX_BUF - 1); // All
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
    if (received < 0 && gnutls_error_is_fatal (received) == 0) // All
    {
      if (_debug)
        std::cout << "c: WARNING " << gnutls_strerror (received) << '\n'; // All
    }
    else if (received < 0)
      throw std::string (gnutls_strerror (received)); // All

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
