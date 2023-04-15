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

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <TCPServer.h>
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
#define HEADER_SIZE 4
#define MAX_BUF 16384

////////////////////////////////////////////////////////////////////////////////
TCPServer::TCPServer ()
{
}

////////////////////////////////////////////////////////////////////////////////
TCPServer::~TCPServer ()
{
  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TCPServer::queue (int depth)
{
  _queue = depth;
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TCPServer::debug (int level)
{
  if (level)
    _debug = true;
}

////////////////////////////////////////////////////////////////////////////////
void TCPServer::bind (
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
void TCPServer::listen ()
{
  if (::listen (_socket, _queue) < 0)
    throw std::string (::strerror (errno));

  if (_debug)
    std::cout << "s: INFO Server listening.\n";
}

////////////////////////////////////////////////////////////////////////////////
void TCPServer::accept (TCPTransaction& tx)
{
  if (_debug)
    tx.debug ();

  struct sockaddr_in sa_cli {};
  socklen_t client_len = sizeof sa_cli;
  int conn;
  do
  {
    conn = ::accept (_socket, (struct sockaddr *) &sa_cli, &client_len);
  }
  while (errno == EINTR);

  if (conn < 0)
    throw std::string (::strerror (errno));

  tx.accept (conn, (struct sockaddr *)&sa_cli);
}

////////////////////////////////////////////////////////////////////////////////
TCPTransaction::~TCPTransaction ()
{
  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
    _socket = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::accept (int socket, struct sockaddr *sa_remote)
{
  _socket = socket;

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
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 's: ...'.
void TCPTransaction::debug ()
{
  _debug = true;
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::limit (int max)
{
  _limit = max;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::send (const std::string& data)
{
  std::string packet = "XXXX" + data;

  // Encode the length.
  unsigned long l = packet.length ();
  packet[0] = l >>24;
  packet[1] = l >>16;
  packet[2] = l >>8;
  packet[3] = l;

  size_t total = 0;

  try
  {
    total = do_send (packet.c_str (), packet.length ());
  }
  catch (...) {}

  if (_debug)
    std::cout << "s: INFO Sending 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
size_t TCPTransaction::do_send(const void *data, size_t len)
{
  const char *buf = static_cast<const char *>(data);
  size_t total = 0;

  int status;
  do
  {
    errno = 0;
    status = ::send (_socket, buf + total, len - total, MSG_NOSIGNAL);
  }
  while ((status > 0 && (total += status) < len) ||
          errno == EINTR || errno == EAGAIN);

  if (status < 0)
  {
    throw std::string (strerror (errno));
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::recv (std::string& data)
{
  data = "";          // No appending of data.
  size_t total = 0;

  // Get the encoded length.
  unsigned char header[HEADER_SIZE] {};
  try
  {
    total = do_recv (header, HEADER_SIZE);
  }
  catch (std::string e) { throw std::string ("Failed to receive header: ") + e; }

  if (total < HEADER_SIZE) {
    throw std::string ("Failed to receive header: connection lost?");
  }

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];
  if (_debug)
    std::cout << "s: INFO expecting " << expected << " bytes.\n";

  if (_limit && expected >= (unsigned long) _limit) {
    std::ostringstream err_str;
    err_str << "Expected message size " << expected << " is larger than allowed limit " << _limit;
    throw err_str.str ();
  }

  data.resize (expected - total);
  total = do_recv (&data[0], data.length ());

  // Other end closed the connection.
  if (total < data.length ())
  {
    if (_debug)
      std::cout << "s: INFO Peer has closed the TCP connection\n";
  }

  if (_debug)
    std::cout << "s: INFO Receiving 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
size_t TCPTransaction::do_recv (void *buffer, size_t len)
{
  char *buf = static_cast<char *>(buffer);
  size_t total = 0;

  int received;
  do
  {
    errno = 0;
    received = ::recv (_socket, buf + total, len - total, MSG_WAITALL);
  }
  while ((received > 0 && (total += received) < HEADER_SIZE) ||
          errno == EINTR || errno == EAGAIN);

  if (received < 0)
  {
    throw std::string (strerror (errno));
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::getClient (std::string& address, int& port)
{
  address = _address;
  port = _port;
}

////////////////////////////////////////////////////////////////////////////////
