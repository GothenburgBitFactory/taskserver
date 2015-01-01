////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

class TLSTransaction;

class TLSServer
{
public:
  enum trust_level { strict, allow_all };

  TLSServer ();
  ~TLSServer ();
  void queue (int);
  void debug (int);
  enum trust_level trust () const;
  void trust (const enum trust_level);
  void ciphers (const std::string&);
  void init (const std::string&, const std::string&, const std::string&, const std::string&);
  void bind (const std::string&, const std::string&);
  void listen ();
  void accept (TLSTransaction&);

  friend class TLSTransaction;

private:
  std::string                      _ca;
  std::string                      _crl;
  std::string                      _cert;
  std::string                      _key;
  std::string                      _ciphers;
  gnutls_certificate_credentials_t _credentials;
  gnutls_dh_params_t               _params;
  gnutls_priority_t                _priorities;
  int                              _socket;
  int                              _queue;
  bool                             _debug;
  enum trust_level                 _trust;
};

class TLSTransaction
{
public:
  TLSTransaction ();
  ~TLSTransaction ();
  void init (TLSServer&);
  void bye ();
  void debug ();
  void trust (const enum TLSServer::trust_level);
  void limit (int);
  int verify_certificate () const;
  void send (const std::string&);
  void recv (std::string&);
  void getClient (std::string&, int&);

private:
  int                         _socket;
  gnutls_session_t            _session;
  int                         _limit;
  bool                        _debug;
  std::string                 _address;
  int                         _port;
  enum TLSServer::trust_level _trust;
};

#endif
#endif

////////////////////////////////////////////////////////////////////////////////

