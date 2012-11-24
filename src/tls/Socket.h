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
#ifndef INCLUDED_SOCKET
#define INCLUDED_SOCKET

#include <string>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// TODO Restore.
//#include <cmake.h>

class Socket
{
public:
  Socket ();
  Socket (int);
  ~Socket ();

  // Client
  void connect (const std::string&, const std::string&);
#ifdef HAVE_LIBGNUTLS
  void ca_cert (const std::string&);
  void crl (const std::string&);
  void cert (const std::string&);
#endif

  // Server
  void bind (const std::string&);
  void listen (int queue = 5);
  int accept ();
  void read (std::string&);
  void write (const std::string&);

  void close ();

  void limit (int);
  void debug ();

private:
  void* get_in_addr (struct sockaddr*);

private:
  int  _socket;
  int  _limit;
  bool _debug;

#ifdef HAVE_LIBGNUTLS
  std::string _ca_cert;
  std::string _crl;
  std::string _cert;
#endif
};

#endif

////////////////////////////////////////////////////////////////////////////////
