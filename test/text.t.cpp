////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2016, Göteborg Bit Factory.
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
#include <text.h>
#include <utf8.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (38);

  // std::string unquoteText (const std::string& text)
  t.is (unquoteText (""),         "",     "unquoteText '' -> ''");
  t.is (unquoteText ("x"),        "x",    "unquoteText 'x' -> 'x'");
  t.is (unquoteText ("'x"),       "'x",   "unquoteText ''x' -> ''x'");
  t.is (unquoteText ("x'"),       "x'",   "unquoteText 'x'' -> 'x''");
  t.is (unquoteText ("\"x"),      "\"x",  "unquoteText '\"x' -> '\"x'");
  t.is (unquoteText ("x\""),      "x\"",  "unquoteText 'x\"' -> 'x\"'");
  t.is (unquoteText ("''"),       "",     "unquoteText '''' -> ''");
  t.is (unquoteText ("'''"),      "'",    "unquoteText ''''' -> '''");
  t.is (unquoteText ("\"\""),     "",     "unquoteText '\"\"' -> ''");
  t.is (unquoteText ("\"\"\""),    "\"",  "unquoteText '\"\"\"' -> '\"'");
  t.is (unquoteText ("''''"),     "''",   "unquoteText '''''' -> ''''");
  t.is (unquoteText ("\"\"\"\""), "\"\"", "unquoteText '\"\"\"\"' -> '\"\"'");
  t.is (unquoteText ("'\"\"'"),   "\"\"", "unquoteText '''\"\"' -> '\"\"'");
  t.is (unquoteText ("\"''\""),   "''",   "unquoteText '\"''\"' -> ''''");
  t.is (unquoteText ("'x'"),      "x",    "unquoteText ''x'' -> 'x'");
  t.is (unquoteText ("\"x\""),    "x",    "unquoteText '\"x\"' -> 'x'");

  // std::string commify (const std::string& data)
  t.is (commify (""),           "",              "commify '' -> ''");
  t.is (commify ("1"),          "1",             "commify '1' -> '1'");
  t.is (commify ("12"),         "12",            "commify '12' -> '12'");
  t.is (commify ("123"),        "123",           "commify '123' -> '123'");
  t.is (commify ("1234"),       "1,234",         "commify '1234' -> '1,234'");
  t.is (commify ("12345"),      "12,345",        "commify '12345' -> '12,345'");
  t.is (commify ("123456"),     "123,456",       "commify '123456' -> '123,456'");
  t.is (commify ("1234567"),    "1,234,567",     "commify '1234567' -> '1,234,567'");
  t.is (commify ("12345678"),   "12,345,678",    "commify '12345678' -> '12,345,678'");
  t.is (commify ("123456789"),  "123,456,789",   "commify '123456789' -> '123,456,789'");
  t.is (commify ("1234567890"), "1,234,567,890", "commify '1234567890' -> '1,234,567,890'");

  t.is (commify ("pre"),         "pre",          "commify 'pre' -> 'pre'");
  t.is (commify ("pre1234"),     "pre1,234",     "commify 'pre1234' -> 'pre1,234'");
  t.is (commify ("1234post"),    "1,234post",    "commify '1234post' -> '1,234post'");
  t.is (commify ("pre1234post"), "pre1,234post", "commify 'pre1234post' -> 'pre1,234post'");

  // int damerau_levenshtein (const char*, const char*);
  t.is    (damerau_levenshtein ("foo", "foo"),  0, "foo --> foo = 0");
  t.is    (damerau_levenshtein ("foo", "food"), 1, "foo --> food = 1");
  t.is    (damerau_levenshtein ("ffoo", "foo"), 1, "ffoo --> foo = 1");
  t.is    (damerau_levenshtein ("foo", "fox"),  1, "foo --> fox = 1");
  t.is    (damerau_levenshtein ("foo", "ofo"),  1, "foo --> ofo = 1");
  t.is    (damerau_levenshtein ("one", "two"),  3, "one --> two = 3");
  t.is    (damerau_levenshtein ("wonderfulmacintosh", "winchestermachine"), 12, "wonderfulmacintosh --> winchestermachine = 12");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
