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

static const size_t MAX_BUF = 16384;

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

  struct sockaddr_storage sa_cli {};
  socklen_t client_len = sizeof sa_cli;
  int conn;
  do
  {
    conn = ::accept (_socket, (struct sockaddr *) &sa_cli, &client_len);
  }
  while (errno == EINTR);

  if (conn < 0)
    throw std::string (::strerror (errno));

  tx.accept (conn, (struct sockaddr *) &sa_cli);
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
  const void *in_addr;
  switch (sa_remote->sa_family) {
  case AF_INET:
    in_addr = &((sockaddr_in *)sa_remote)->sin_addr;
    _port = ntohs (((sockaddr_in *)sa_remote)->sin_port);
    break;
  case AF_INET6:
    in_addr = &((sockaddr_in6 *)sa_remote)->sin6_addr;
    _port = ntohs (((sockaddr_in6 *)sa_remote)->sin6_port);
    break;
  default:
    throw "BUG: got unknown remote address family from accept()";
  }

  char topbuf[512];
  _address = inet_ntop (sa_remote->sa_family, in_addr, topbuf, sizeof (topbuf));

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

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::send (const std::string& data)
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
    int status = do_send (packet.c_str () + total, remaining);

    if (status < 0)
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
size_t TCPTransaction::recv_block (void *bufp, size_t len)
{
  char *buf = (char *)bufp;
  size_t remaining = len;

  while (remaining > 0) {
    ssize_t n = do_recv (buf, remaining);

    // Other end closed the connection.
    if (n == 0)
    {
      if (_debug)
        std::cout << "s: INFO Peer has closed the TCP connection\n";
      return 0;
    }

    buf += n;
    remaining -= n;
  }

  return len;
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::recv (std::string& data)
{
  data = "";          // No appending of data.
  ssize_t received;

  // Get the encoded length.
  unsigned char header[4] {};
  received = recv_block (header, 4);
  if (received == 0)
    return;

  size_t total = received;

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];
  if (_debug)
    std::cout << "s: INFO expecting " << expected << " bytes.\n";

  if (_limit && expected > _limit)
    throw std::string("client tries to send too big a message");

  // Arbitrary buffer size.
  char buffer[MAX_BUF];

  // Keep reading until no more data.  Concatenate chunks of data if a) the
  // read was interrupted by a signal, and b) if there is more data than
  // fits in the buffer.
  while (total < expected)
  {
    received = recv_block (buffer, std::min (expected - total, MAX_BUF));

    // Other end closed the connection.
    if (received == 0)
      return;

    data.append(buffer, received);
    total += received;
  }

  if (_debug)
    std::cout << "s: INFO Receiving 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void TCPTransaction::getClient (std::string& address, int& port)
{
  address = _address;
  port = _port;
}

////////////////////////////////////////////////////////////////////////////////
ssize_t TCPTransaction::do_send (const void *buf, size_t len)
{
  ssize_t status;
  do
  {
    status = ::send (_socket, buf, len, MSG_NOSIGNAL);
  }
  while (status < 0 && (errno == EINTR || errno == EAGAIN));

  return status < 0 ? -errno : status;
}

////////////////////////////////////////////////////////////////////////////////
ssize_t TCPTransaction::do_recv (void *buf, size_t len)
{
  ssize_t received;

  do
  {
    received = ::recv (_socket, buf, len, MSG_WAITALL);
    if (received < 0 && errno != EINTR && errno != EAGAIN)
      throw std::string (::strerror (errno));
  }
  while (received < 0);

  return received;
}

////////////////////////////////////////////////////////////////////////////////
