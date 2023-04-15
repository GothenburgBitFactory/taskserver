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
#ifndef INCLUDED_TCPSERVER
#define INCLUDED_TCPSERVER

#include <string>

class TCPTransaction;

class TCPServer
{
public:
  TCPServer ();
  ~TCPServer ();
  void queue (int);
  virtual void debug (int);
  void bind (const std::string&, const std::string&, const std::string&);
  void listen ();
  void accept (TCPTransaction&);

protected:
  bool                             _debug       {false};

private:
  int                              _socket      {0};
  int                              _queue       {5};
};

class TCPTransaction
{
public:
  TCPTransaction () = default;
  ~TCPTransaction ();
  virtual void accept (int, struct sockaddr *);
  void debug ();
  void limit (int);
  void send (const std::string&);
  void recv (std::string&);
  void getClient (std::string&, int&);

protected:
  virtual size_t do_send (const void *, size_t);
  virtual size_t do_recv (void *, size_t);

  int                         _socket  {0};
  int                         _limit   {0};
  bool                        _debug   {false};
  std::string                 _address {""};
  int                         _port    {0};
};

#endif

////////////////////////////////////////////////////////////////////////////////
