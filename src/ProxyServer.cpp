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
#include <ProxyServer.h>

// PROXY protocol as specified in https://www.haproxy.org/download/1.8/doc/proxy-protocol.txt

static const char PROXYv2_SIG[12] = { '\r', '\n', '\r', '\n', 0, '\r', '\n', 'Q', 'U', 'I', 'T', '\n' };
static const size_t PROXYv2_HEADER_LEN = 16;
static const size_t PROXYv2_ADDRv4_LEN = 12;
static const size_t PROXYv2_ADDRv6_LEN = 36;

////////////////////////////////////////////////////////////////////////////////
void ProxyTransaction::accept (int socket, struct sockaddr *sa_remote)
{
  _socket = socket;

  char buf[0x10000];

  if (! process_proxy_v2(sa_remote, buf))
    throw "bad PROXYv2 signature received";

  TCPTransaction::accept (socket, sa_remote);
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
  if (do_recv (buf, PROXYv2_HEADER_LEN) < PROXYv2_HEADER_LEN)
    throw "EOF before PROXYv2 signature";
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
