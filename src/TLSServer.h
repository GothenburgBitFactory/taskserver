////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_TLSSERVER
#define INCLUDED_TLSSERVER

#ifdef HAVE_LIBGNUTLS

#include <string>
#include <gnutls/gnutls.h>
#include "TCPServer.h"

class TLSTransaction;

class TLSServer : public TCPServer
{
public:
  enum trust_level { strict, allow_all };

  TLSServer ();
  ~TLSServer ();
  void debug (int) override;
  enum trust_level trust () const;
  void trust (const enum trust_level);
  void ciphers (const std::string&);
  void dh_bits (unsigned int dh_bits);
  void init (const std::string&, const std::string&, const std::string&, const std::string&);

  friend class TLSTransaction;

private:
  std::string                      _ca          {""};
  std::string                      _crl         {""};
  std::string                      _cert        {""};
  std::string                      _key         {""};
  std::string                      _ciphers     {""};
  unsigned int                     _dh_bits     {0};
  gnutls_certificate_credentials_t _credentials {};
  gnutls_priority_t                _priorities  {};
  enum trust_level                 _trust       {TLSServer::strict};
  bool                             _priorities_init {false};
};

class TLSTransaction : public TCPTransaction
{
public:
  TLSTransaction (TLSServer&);
  ~TLSTransaction ();
  void accept (int, struct sockaddr *) override;
  void bye ();
  void trust (const enum TLSServer::trust_level);
  int verify_certificate () const;

protected:
  size_t do_send (const void *, size_t) override;
  size_t do_recv (void *, size_t) override;

private:
  gnutls_session_t            _session {};
  enum TLSServer::trust_level _trust   {TLSServer::strict};
};

#endif
#endif

////////////////////////////////////////////////////////////////////////////////

