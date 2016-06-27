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
  UnitTest t (69);

  // void split (std::vector<std::string>& results, const std::string& input, const char delimiter)
  std::vector <std::string> items;
  std::string unsplit = "";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 0, "split '' '-' -> 0 items");

  unsplit = "a";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  unsplit = "-";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 2, "split '-' '-' -> '' ''");
  t.is (items[0], "",              "split '-' '-' -> [0] ''");
  t.is (items[1], "",              "split '-' '-' -> [1] ''");

  unsplit = "-a-bc-def";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 4, "split '-a-bc-def' '-' -> '' 'a' 'bc' 'def'");
  t.is (items[0], "",              "split '-a-bc-def' '-' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc-def' '-' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc-def' '-' -> [2] 'bc'");
  t.is (items[3], "def",           "split '-a-bc-def' '-' -> [3] 'def'");

  // void split (std::vector<std::string>& results, const std::string& input, const std::string& delimiter)
  unsplit = "";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 0, "split '' '--' -> 0 items");

  unsplit = "a";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 1, "split 'a' '--' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  unsplit = "--";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 2, "split '-' '--' -> '' ''");
  t.is (items[0], "",              "split '-' '-' -> [0] ''");
  t.is (items[1], "",              "split '-' '-' -> [1] ''");

  unsplit = "--a--bc--def";
  split (items, unsplit, "--");
  t.is (items.size (), (size_t) 4, "split '-a-bc-def' '--' -> '' 'a' 'bc' 'def'");
  t.is (items[0], "",              "split '-a-bc-def' '--' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc-def' '--' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc-def' '--' -> [2] 'bc'");
  t.is (items[3], "def",           "split '-a-bc-def' '--' -> [3] 'def'");

  unsplit = "one\ntwo\nthree";
  split (items, unsplit, "\n");
  t.is (items.size (), (size_t) 3, "split 'one\\ntwo\\nthree' -> 'one', 'two', 'three'");
  t.is (items[0], "one",           "split 'one\\ntwo\\nthree' -> [0] 'one'");
  t.is (items[1], "two",           "split 'one\\ntwo\\nthree' -> [1] 'two'");
  t.is (items[2], "three",         "split 'one\\ntwo\\nthree' -> [2] 'three'");

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

  // int utf8_length (const std::string&);
  t.is ((int) utf8_length ("Çirçös"),            6, "utf8_length (Çirçös) == 6");
  t.is ((int) utf8_length ("ツネナラム"),        5, "utf8_length (ツネナラム) == 5");
  t.is ((int) utf8_length ("Zwölf Boxkämpfer"), 16, "utf8_length (Zwölf Boxkämpfer) == 16");

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
