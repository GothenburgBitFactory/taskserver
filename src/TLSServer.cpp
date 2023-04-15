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
#include <sstream>
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
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TLSServer::debug (int level)
{
  TCPServer::debug (level);

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
TLSTransaction::~TLSTransaction ()
{
  gnutls_deinit (_session); // All
}

////////////////////////////////////////////////////////////////////////////////
TLSTransaction::TLSTransaction (TLSServer& server)
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

  trust (server.trust ());
}

void TLSTransaction::accept (int socket, struct sockaddr *sa_remote)
{
  TCPTransaction::accept (socket, sa_remote);

#if GNUTLS_VERSION_NUMBER >= 0x030109
  gnutls_transport_set_int (_session, _socket); // 3.1.9
#else
  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) (intptr_t) _socket); // All
#endif

  // Perform the TLS handshake
  int ret;
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
      throw format ("Handshake failed for host '{2}'. {1}", error, _address);
    }
#else
    throw format ("Handshake failed for host '{2}'. {1}", gnutls_strerror (ret), _address); // All
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
void TLSTransaction::trust (const enum TLSServer::trust_level value)
{
  _trust = value;
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
size_t TLSTransaction::do_send(const void *data, size_t len)
{
  const char *buf = static_cast<const char *>(data);
  size_t total = 0;

  int status;
  do
  {
    status = gnutls_record_send (_session, buf + total, len - total); // All
  }
  while ((status > 0 && (total += status) < len) ||
          status == GNUTLS_E_INTERRUPTED ||
          status == GNUTLS_E_AGAIN);

  if (status < 0)
  {
    throw std::string (gnutls_strerror (errno));
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
size_t TLSTransaction::do_recv (void *buffer, size_t len)
{
  char *buf = static_cast<char *>(buffer);
  size_t total = 0;

  int received;
  do
  {
      received = gnutls_record_recv (_session, buf + total, len - total);
  }
  while ((received > 0 && (total += received) < len) ||
          received == GNUTLS_E_INTERRUPTED ||
          received == GNUTLS_E_AGAIN);

  if (received < 0)
  {
    throw std::string (gnutls_strerror (errno));
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
#endif
