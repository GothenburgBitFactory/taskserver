////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2015, Göteborg Bit Factory.
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
  UnitTest t (177);

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

  split_minimal (items, unsplit, '-');
  t.is (items.size (), (size_t) 0, "split '-' '-' ->");

  unsplit = "-a-bc-def";
  split (items, unsplit, '-');
  t.is (items.size (), (size_t) 4, "split '-a-bc-def' '-' -> '' 'a' 'bc' 'def'");
  t.is (items[0], "",              "split '-a-bc-def' '-' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc-def' '-' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc-def' '-' -> [2] 'bc'");
  t.is (items[3], "def",           "split '-a-bc-def' '-' -> [3] 'def'");

  split_minimal (items, unsplit, '-');
  t.is (items.size (), (size_t) 3, "split '-a-bc-def' '-' -> 'a' 'bc' 'def'");
  t.is (items[0], "a",             "split '-a-bc-def' '-' -> [1] 'a'");
  t.is (items[1], "bc",            "split '-a-bc-def' '-' -> [2] 'bc'");
  t.is (items[2], "def",           "split '-a-bc-def' '-' -> [3] 'def'");

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

  // void join (std::string& result, const std::string& separator, const std::vector<std::string>& items)
  std::vector <std::string> unjoined;
  std::string joined;

  join (joined, "", unjoined);
  t.is (joined.length (), (size_t) 0,  "join -> length 0");
  t.is (joined,           "",          "join -> ''");

  unjoined = {"", "a", "bc", "def"};
  join (joined, "", unjoined);
  t.is (joined.length (), (size_t) 6, "join '' 'a' 'bc' 'def' -> length 6");
  t.is (joined,           "abcdef",   "join '' 'a' 'bc' 'def' -> 'abcdef'");

  join (joined, "-", unjoined);
  t.is (joined.length (), (size_t) 9,  "join '' - 'a' - 'bc' - 'def' -> length 9");
  t.is (joined,           "-a-bc-def", "join '' - 'a' - 'bc' - 'def' -> '-a-bc-def'");

  // void join (std::string& result, const std::string& separator, const std::vector<int>& items)
  std::vector <int> unjoined2;

  join (joined, "", unjoined2);
  t.is (joined.length (), (size_t) 0, "join -> length 0");
  t.is (joined,           "",         "join -> ''");

  unjoined2 = {0, 1, 2};
  join (joined, "", unjoined2);
  t.is (joined.length (), (size_t) 3, "join 0 1 2 -> length 3");
  t.is (joined,           "012",      "join 0 1 2 -> '012'");

  join (joined, "-", unjoined2);
  t.is (joined.length (), (size_t) 5, "join 0 1 2 -> length 5");
  t.is (joined,           "0-1-2",    "join 0 1 2 -> '0-1-2'");

  // std::string trimLeft (const std::string& in, const std::string& t /*= " "*/)
  t.is (trimLeft (""),                     "",            "trimLeft '' -> ''");
  t.is (trimLeft ("   "),                  "",            "trimLeft '   ' -> ''");
  t.is (trimLeft ("",              " \t"), "",            "trimLeft '' -> ''");
  t.is (trimLeft ("xxx"),                  "xxx",         "trimLeft 'xxx' -> 'xxx'");
  t.is (trimLeft ("xxx",           " \t"), "xxx",         "trimLeft 'xxx' -> 'xxx'");
  t.is (trimLeft ("  \t xxx \t  "),        "\t xxx \t  ", "trimLeft '  \\t xxx \\t  ' -> '\\t xxx \\t  '");
  t.is (trimLeft ("  \t xxx \t  ", " \t"), "xxx \t  ",    "trimLeft '  \\t xxx \\t  ' -> 'xxx \\t  '");

  // std::string trimRight (const std::string& in, const std::string& t /*= " "*/)
  t.is (trimRight (""),                     "",            "trimRight '' -> ''");
  t.is (trimRight ("   "),                  "",            "trimRight '   ' -> ''");
  t.is (trimRight ("",              " \t"), "",            "trimRight '' -> ''");
  t.is (trimRight ("xxx"),                  "xxx",         "trimRight 'xxx' -> 'xxx'");
  t.is (trimRight ("xxx",           " \t"), "xxx",         "trimRight 'xxx' -> 'xxx'");
  t.is (trimRight ("  \t xxx \t  "),        "  \t xxx \t", "trimRight '  \\t xxx \\t  ' -> '  \\t xxx \\t'");
  t.is (trimRight ("  \t xxx \t  ", " \t"), "  \t xxx",    "trimRight '  \\t xxx \\t  ' -> '  \\t xxx'");

  // std::string trim (const std::string& in, const std::string& t /*= " "*/)
  t.is (trim (""),                     "",          "trim '' -> ''");
  t.is (trim ("   "),                  "",          "trim '   ' -> ''");
  t.is (trim ("",              " \t"), "",          "trim '' -> ''");
  t.is (trim ("xxx"),                  "xxx",       "trim 'xxx' -> 'xxx'");
  t.is (trim ("xxx",           " \t"), "xxx",       "trim 'xxx' -> 'xxx'");
  t.is (trim ("  \t xxx \t  "),        "\t xxx \t", "trim '  \\t xxx \\t  ' -> '\\t xxx \\t'");
  t.is (trim ("  \t xxx \t  ", " \t"), "xxx",       "trim '  \\t xxx \\t  ' -> 'xxx'");

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

  // std::string lowerCase (const std::string& input)
  t.is (lowerCase (""),            "",            "lowerCase '' -> ''");
  t.is (lowerCase ("pre01_:POST"), "pre01_:post", "lowerCase 'pre01_:POST' -> 'pre01_:post'");

  // std::string upperCase (const std::string& input)
  t.is (upperCase (""),            "",            "upperCase '' -> ''");
  t.is (upperCase ("pre01_:POST"), "PRE01_:POST", "upperCase 'pre01_:POST' -> 'PRE01_:POST'");

  // bool digitsOnly (const std::string&);
  t.ok    (digitsOnly (""),                       "digitsOnly '' -> true");
  t.ok    (digitsOnly ("0"),                      "digitsOnly '0' -> true");
  t.ok    (digitsOnly ("123"),                    "digitsOnly '123' -> true");
  t.notok (digitsOnly ("12fa"),                   "digitsOnly '12fa' -> false");

  std::string text = "Hello, world.";
  //      0123456789012
  //      s   e  s   e

  // bool compare (const std::string&, const std::string&, bool caseless = false);
  // Make sure degenerate cases are handled.
  t.ok    (compare ("", ""),    "'' == ''");
  t.notok (compare ("foo", ""), "foo != ''");
  t.notok (compare ("", "foo"), "'' != foo");

  // Make sure the default is case-sensitive.
  t.ok    (compare ("foo", "foo"), "foo == foo");
  t.notok (compare ("foo", "FOO"), "foo != foo");

  // Test case-sensitive.
  t.notok (compare ("foo", "xx", true),  "foo != xx");

  t.ok    (compare ("foo", "foo", true), "foo == foo");
  t.notok (compare ("foo", "FOO", true), "foo != FOO");
  t.notok (compare ("FOO", "foo", true), "FOO != foo");
  t.ok    (compare ("FOO", "FOO", true), "FOO == FOO");

  // Test case-insensitive.
  t.notok (compare ("foo", "xx", false),   "foo != foo (caseless)");

  t.ok    (compare ("foo", "foo", false),  "foo == foo (caseless)");
  t.ok    (compare ("foo", "FOO", false),  "foo == FOO (caseless)");
  t.ok    (compare ("FOO", "foo", false),  "FOO == foo (caseless)");
  t.ok    (compare ("FOO", "FOO", false),  "FOO == FOO (caseless)");

  // std::string::size_type find (const std::string&, const std::string&, bool caseless = false);
  // Make sure degenerate cases are handled.
  t.is ((int) find ("foo", ""), (int) 0,                 "foo !contains ''");
  t.is ((int) find ("", "foo"), (int) std::string::npos, "'' !contains foo");

  // Make sure the default is case-sensitive.
  t.is ((int) find ("foo", "fo"), 0,                       "foo contains fo");
  t.is ((int) find ("foo", "FO"), (int) std::string::npos, "foo !contains fo");

  // Test case-sensitive.
  t.is ((int) find ("foo", "xx", true), (int) std::string::npos, "foo !contains xx");
  t.is ((int) find ("foo", "oo", true), 1,                       "foo contains oo");

  t.is ((int) find ("foo", "fo", true), 0,                       "foo contains fo");
  t.is ((int) find ("foo", "FO", true), (int) std::string::npos, "foo !contains fo");
  t.is ((int) find ("FOO", "fo", true), (int) std::string::npos, "foo !contains fo");
  t.is ((int) find ("FOO", "FO", true), 0,                       "foo contains fo");

  // Test case-insensitive.
  t.is ((int) find ("foo", "xx", false),  (int) std::string::npos, "foo !contains xx (caseless)");
  t.is ((int) find ("foo", "oo", false),  1,                       "foo contains oo (caseless)");

  t.is ((int) find ("foo", "fo", false),  0, "foo contains fo (caseless)");
  t.is ((int) find ("foo", "FO", false),  0, "foo contains FO (caseless)");
  t.is ((int) find ("FOO", "fo", false),  0, "FOO contains fo (caseless)");
  t.is ((int) find ("FOO", "FO", false),  0, "FOO contains FO (caseless)");

  // Test start offset.
  t.is ((int) find ("one two three", "e",  3, true), (int) 11, "offset obeyed");
  t.is ((int) find ("one two three", "e", 11, true), (int) 11, "offset obeyed");

  // TODO bool closeEnough (const std::string&, const std::string&);

  // std::string format (char);
  t.is (format ('A'), "A", "format ('A') -> A");

  // std::string format (int);
  t.is (format (0),  "0",  "format (0) -> 0");
  t.is (format (-1), "-1", "format (-1) -> -1");

  // std::string formatHex (int);
  t.is (formatHex (0),   "0",  "formatHex (0) -> 0");
  t.is (formatHex (10),  "a",  "formatHex (10) -> a");
  t.is (formatHex (123), "7b", "formatHex (123) -> 7b");

  // std::string format (float, int, int);
  t.is (format (1.23456789, 8, 1),      "       1",     "format (1.23456789,    8,   1) -> _______1");
  t.is (format (1.23456789, 8, 2),      "     1.2",     "format (1.23456789,    8,   2) -> _____1.2");
  t.is (format (1.23456789, 8, 3),      "    1.23",     "format (1.23456789,    8,   3) -> ____1.23");
  t.is (format (1.23456789, 8, 4),      "   1.235",     "format (1.23456789,    8,   4) -> ___1.235");
  t.is (format (1.23456789, 8, 5),      "  1.2346",     "format (1.23456789,    8,   5) -> __1.2346");
  t.is (format (1.23456789, 8, 6),      " 1.23457",     "format (1.23456789,    8,   6) ->  1.23457");
  t.is (format (1.23456789, 8, 7),      "1.234568",     "format (1.23456789,    8,   7) -> 1.234568");
  t.is (format (1.23456789, 8, 8),      "1.2345679",    "format (1.23456789,    8,   8) -> 1.2345679");
  t.is (format (2444238.56789, 12, 11), "2444238.5679", "format (2444238.56789, 12, 11) -> 2444238.5679");

  // std::string format (double, int, int);

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify (123, 3), "123",   "leftJustify 123,3 -> '123'");
  t.is (leftJustify (123, 4), "123 ",  "leftJustify 123,4 -> '123 '");
  t.is (leftJustify (123, 5), "123  ", "leftJustify 123,5 -> '123  '");

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify ("foo", 3), "foo",   "leftJustify foo,3 -> 'foo'");
  t.is (leftJustify ("foo", 4), "foo ",  "leftJustify foo,4 -> 'foo '");
  t.is (leftJustify ("foo", 5), "foo  ", "leftJustify foo,5 -> 'foo  '");
  t.is (leftJustify ("föo", 5), "föo  ", "leftJustify föo,5 -> 'föo  '");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify (123, 3), "123",   "rightJustify 123,3 -> '123'");
  t.is (rightJustify (123, 4), " 123",  "rightJustify 123,4 -> ' 123'");
  t.is (rightJustify (123, 5), "  123", "rightJustify 123,5 -> '  123'");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify ("foo", 3), "foo",   "rightJustify foo,3 -> 'foo'");
  t.is (rightJustify ("foo", 4), " foo",  "rightJustify foo,4 -> ' foo'");
  t.is (rightJustify ("foo", 5), "  foo", "rightJustify foo,5 -> '  foo'");
  t.is (rightJustify ("föo", 5), "  föo", "rightJustify föo,5 -> '  föo'");

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
