////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2010 - 2013, Göteborg Bit Factory.
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

#include <util.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (12);

  // std::string formatBytes (size_t);
  t.is (formatBytes (0), "0 B", "0 -> 0 B");

  t.is (formatBytes (994),  "994 B", "994 -> 994 B");
  t.is (formatBytes (995),  "1.0 KiB", "995 -> 1.0 KiB");
  t.is (formatBytes (999),  "1.0 KiB", "999 -> 1.0 KiB");
  t.is (formatBytes (1000), "1.0 KiB", "1000 -> 1.0 KiB");
  t.is (formatBytes (1001), "1.0 KiB", "1001 -> 1.0 KiB");

  t.is (formatBytes (999999),  "1.0 MiB", "999999 -> 1.0 MiB");
  t.is (formatBytes (1000000), "1.0 MiB", "1000000 -> 1.0 MiB");
  t.is (formatBytes (1000001), "1.0 MiB", "1000001 -> 1.0 MiB");

  t.is (formatBytes (999999999),  "1.0 GiB", "999999999 -> 1.0 GiB");
  t.is (formatBytes (1000000000), "1.0 GiB", "1000000000 -> 1.0 GiB");
  t.is (formatBytes (1000000001), "1.0 GiB", "1000000001 -> 1.0 GiB");

  // TODO const std::string uuid ();

  // TODO const std::string encode (const std::string& value);
  // TODO const std::string decode (const std::string& value);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

