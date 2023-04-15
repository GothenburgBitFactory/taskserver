////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019, Göteborg Bit Factory.
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

#include <stdint.h>
#include <string.h>
#include <format.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ProxyServer.h>

// PROXY protocol as specified in https://www.haproxy.org/download/1.8/doc/proxy-protocol.txt

static const char PROXYv1_SIG[6] = { 'P', 'R', 'O', 'X', 'Y', ' ' };
static const size_t PROXYv1_MAX_REQ_LEN = 108 - sizeof(PROXYv1_SIG);
static const char PROXYv2_SIG[12] = { '\r', '\n', '\r', '\n', 0, '\r', '\n', 'Q', 'U', 'I', 'T', '\n' };
static const size_t PROXYv2_HEADER_LEN = 16;
static const size_t PROXYv2_ADDRv4_LEN = 12;
static const size_t PROXYv2_ADDRv6_LEN = 36;

////////////////////////////////////////////////////////////////////////////////
void ProxyTransaction::accept (int socket, struct sockaddr *sa_remote)
{
  _socket = socket;

  char buf[0x10000];

  if (! process_proxy_v1(sa_remote, buf))
    if (! process_proxy_v2(sa_remote, buf))
      throw "bad PROXY signature received";

  TCPTransaction::accept (socket, sa_remote);
}

////////////////////////////////////////////////////////////////////////////////
static void parse_proxy_address (char *buf, int af, void *addr, in_port_t& port)
{
  // buf: "SRCIP DSTIP SPORT DPORT"

  char *e = strchr (buf, ' ');
  if (e == NULL)
    throw "bad PROXY request format";
  *e++ = 0;

  if (inet_pton (af, buf, addr) != 1)
    throw "bad PROXY request format";

  buf = strchr (e, ' ');
  if (buf++ == NULL)
    throw "bad PROXY request format";

  e = strchr (buf, ' ');
  if (e == NULL)
    throw "bad PROXY request format";
  *e = 0;

  errno = 0;
  port = strtoul (buf, &e, 10);
  if (!*buf || *e || errno)
    throw "bad PROXY request format";
}

////////////////////////////////////////////////////////////////////////////////
static void parse_proxy_ipv4 (struct sockaddr_in& sa, char *buf)
{
  sa.sin_family = AF_INET;
  parse_proxy_address (buf, AF_INET, &sa.sin_addr, sa.sin_port);
}

////////////////////////////////////////////////////////////////////////////////
static void parse_proxy_ipv6 (struct sockaddr_in6& sa, char *buf)
{
  memset (&sa, 0, sizeof(sa));
  sa.sin6_family = AF_INET6;
  parse_proxy_address (buf, AF_INET6, &sa.sin6_addr, sa.sin6_port);
}

////////////////////////////////////////////////////////////////////////////////
bool ProxyTransaction::recv_line (char *buf, size_t max_len)
{
  size_t offs = 0;
  do
  {
    do
    {
      if (do_recv (buf + offs, 2) < 2)
        return false;
      offs += 2;
    }
    while (offs < max_len && buf[offs - 1] != '\n' && buf[offs - 1] != '\r');

    if (buf[offs - 1] == '\r')
    {
      if (do_recv (buf + offs++, 1) == 0)
        return false;
    }
  }
  while (offs < max_len && buf[offs - 1] != '\n');

  if (buf[--offs] != '\n')
    throw "PROXY request too big";
  buf[--offs] = 0;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ProxyTransaction::process_proxy_v1 (struct sockaddr *sa_remote, char *buf)
{
  if (do_recv (buf, sizeof(PROXYv1_SIG)) < sizeof(PROXYv1_SIG))
    throw "EOF before PROXY signature";
  if (memcmp (buf, PROXYv1_SIG, sizeof(PROXYv1_SIG)))
    return false;

  if (!recv_line (buf, PROXYv1_MAX_REQ_LEN))
    throw "EOF in PROXY request";

  if (!strncmp (buf, "UNKNOWN", 7))
    sa_remote->sa_family = AF_UNSPEC;
  else if (!strncmp (buf, "TCP4 ", 5))
    parse_proxy_ipv4 (*(struct sockaddr_in *)sa_remote, buf + 5);
  else if (!strncmp (buf, "TCP6 ", 5))
    parse_proxy_ipv6 (*(struct sockaddr_in6 *)sa_remote, buf + 5);
  else
    throw "invalid PROXY request protocol";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static void fill_proxy_ipv4(struct sockaddr_in& sa, char *& buf, size_t& hlen)
{
  if (hlen < PROXYv2_ADDRv4_LEN)
    throw format ("too short PROXY address for IPv4 ({1} bytes)", hlen);
  sa.sin_family = AF_INET;
  memcpy (&sa.sin_port, buf + 8, 2);
  memcpy (&sa.sin_addr, buf + 0, 4);
  buf += PROXYv2_ADDRv4_LEN;
  hlen -= PROXYv2_ADDRv4_LEN;
}

////////////////////////////////////////////////////////////////////////////////
static void fill_proxy_ipv6(struct sockaddr_in6& sa, char *& buf, size_t& hlen)
{
  if (hlen < PROXYv2_ADDRv6_LEN)
    throw format ("too short PROXY address for IPv6 ({1} bytes)", hlen);
  memset (&sa, 0, sizeof(sa));
  sa.sin6_family = AF_INET6;
  memcpy (&sa.sin6_port, buf + 32, 2);
  memcpy (&sa.sin6_addr, buf + 0, 16);
  buf += PROXYv2_ADDRv6_LEN;
  hlen -= PROXYv2_ADDRv6_LEN;
}

////////////////////////////////////////////////////////////////////////////////
bool ProxyTransaction::process_proxy_v2 (struct sockaddr *sa_remote, char *buf)
{
  static const size_t PROXYv2_HEADER_REMAINDER = PROXYv2_HEADER_LEN - sizeof(PROXYv1_SIG);
  if (do_recv (buf + sizeof(PROXYv1_SIG), PROXYv2_HEADER_REMAINDER) < PROXYv2_HEADER_REMAINDER)
    throw "EOF in PROXYv2 signature";
  if (memcmp (buf, PROXYv2_SIG, sizeof(PROXYv2_SIG)))
    return false;

  char proxycmd = buf[12];
  char afproto = buf[13];
  if (proxycmd != 0x20 && proxycmd != 0x21)
    throw format ("unsupported PROXYv2 protocol+command byte {1}", formatHex (proxycmd));

  size_t hlen = ((unsigned char)buf[14] << 8) | (unsigned char)buf[15];

  if (hlen > 0)
  {
    if (do_recv (buf, hlen) < hlen)
      throw "EOF in PROXYv2 header";
  }

  switch (afproto) {
  case 0x00:
    sa_remote->sa_family = AF_UNSPEC;
    break;
  case 0x11: /* IPv4/TCP */
    fill_proxy_ipv4 (*(struct sockaddr_in *)sa_remote, buf, hlen);
    break;
  case 0x21: /* IPv6/TCP */
    fill_proxy_ipv6 (*(struct sockaddr_in6 *)sa_remote, buf, hlen);
    break;
  default:
    throw format ("unsupported PROXYv2 original protocol byte {1}", formatHex (afproto));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
