////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

#include <TLSClient.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUF 1024

////////////////////////////////////////////////////////////////////////////////
TLSClient::TLSClient ()
: _ca ("")
, _socket (0)
{
}

////////////////////////////////////////////////////////////////////////////////
TLSClient::~TLSClient ()
{
  gnutls_deinit (_session);
  gnutls_certificate_free_credentials (_credentials);
  gnutls_global_deinit ();

  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::init (const std::string& ca)
{
  _ca = ca;

  gnutls_global_init ();
  gnutls_certificate_allocate_credentials (&_credentials);
  gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM);
  gnutls_init (&_session, GNUTLS_CLIENT);

  // Use default priorities.
  const char *err;
  int ret = gnutls_priority_set_direct (_session, "NORMAL", &err);
  if (ret < 0)
  {
    if (ret == GNUTLS_E_INVALID_REQUEST)
      fprintf (stderr, "c: Syntax error at: %s\n", err);

    exit (1);
  }

  // Apply the x509 credentials to the current session.
  gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, _credentials);
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::connect (const std::string& server, const std::string& port)
{
  // connect to server
  _socket = socket (AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sa;
  memset (&sa, '\0', sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (atoi (port.c_str ()));
  inet_pton (AF_INET, server.c_str (), &sa.sin_addr);

  int error = ::connect (_socket, (struct sockaddr *) &sa, sizeof (sa));
  if (error < 0)
  {
    fprintf (stderr, "c: Connect error\n");
    exit (1);
  }

  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) _socket);

  // Perform the TLS handshake
  int ret = gnutls_handshake (_session);

  if (ret < 0)
  {
    fprintf (stderr, "c: *** Handshake failed\n");
    gnutls_perror (ret);
  }
  else
  {
    printf ("c: - Handshake was completed\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::send (const std::string& text)
{
  gnutls_record_send (_session, text.c_str (), text.length ());
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::recv (std::string& text)
{
  char buffer[MAX_BUF + 1];
  int ret = gnutls_record_recv (_session, buffer, MAX_BUF);
  if (ret == 0)
    printf ("c: - Peer has closed the TLS connection\n");
  else if (ret < 0)
    fprintf (stderr, "c: *** Error: %s\n", gnutls_strerror (ret));
  else
  {
    printf ("c: - Received %d bytes: ", ret);
    for (int ii = 0; ii < ret; ii++)
      fputc (buffer[ii], stdout);
    fputs ("\n", stdout);
    text = buffer;

    gnutls_bye (_session, GNUTLS_SHUT_RDWR);
  }
}

////////////////////////////////////////////////////////////////////////////////
