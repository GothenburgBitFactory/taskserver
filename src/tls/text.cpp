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

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <text.h>

static void replace_positional (std::string&, const std::string&, const std::string&);

////////////////////////////////////////////////////////////////////////////////
void split (
  std::vector<std::string>& results,
  const std::string& input,
  const char delimiter)
{
  results.clear ();
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  if (input.length ())
    results.push_back (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void split_minimal (
  std::vector<std::string>& results,
  const std::string& input,
  const char delimiter)
{
  results.clear ();
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    if (i != start)
      results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  if (start < input.length ())
    results.push_back (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void split (
  std::vector<std::string>& results,
  const std::string& input,
  const std::string& delimiter)
{
  results.clear ();
  std::string::size_type length = delimiter.length ();

  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + length;
  }

  if (input.length ())
    results.push_back (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void join (
  std::string& result,
  const std::string& separator,
  const std::vector<std::string>& items)
{
  std::stringstream s;
  unsigned int size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    s << items[i];
    if (i < size - 1)
      s << separator;
  }

  result = s.str ();
}

////////////////////////////////////////////////////////////////////////////////
void join (
  std::string& result,
  const std::string& separator,
  const std::vector<int>& items)
{
  std::stringstream s;
  unsigned int size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    s << items[i];
    if (i < size - 1)
      s << separator;
  }

  result = s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string trimLeft (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return out.erase (0, in.find_first_not_of (t));
}

////////////////////////////////////////////////////////////////////////////////
std::string trimRight (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return out.erase (out.find_last_not_of (t) + 1);
}

////////////////////////////////////////////////////////////////////////////////
std::string trim (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return trimLeft (trimRight (out, t), t);
}

////////////////////////////////////////////////////////////////////////////////
// Remove enclosing balanced quotes.  Assumes trimmed text.
std::string unquoteText (const std::string& input)
{
  std::string output = input;

  if (output.length () > 1)
  {
    char quote = output[0];
    if ((quote == '\'' || quote == '"') &&
        output[output.length () - 1] == quote)
      return output.substr (1, output.length () - 2);
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string commify (const std::string& data)
{
  // First scan for decimal point and end of digits.
  int decimalPoint = -1;
  int end          = -1;

  int i;
  for (int i = 0; i < (int) data.length (); ++i)
  {
    if (isdigit (data[i]))
      end = i;

    if (data[i] == '.')
      decimalPoint = i;
  }

  std::string result;
  if (decimalPoint != -1)
  {
    // In reverse order, transfer all digits up to, and including the decimal
    // point.
    for (i = (int) data.length () - 1; i >= decimalPoint; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }
  else
  {
    // In reverse order, transfer all digits up to, but not including the last
    // digit.
    for (i = (int) data.length () - 1; i > end; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }

  // reverse result into data.
  std::string done;
  for (int i = (int) result.length () - 1; i >= 0; --i)
    done += result[i];

  return done;
}


////////////////////////////////////////////////////////////////////////////////
std::string lowerCase (const std::string& input)
{
  std::string output = input;
  std::transform (output.begin (), output.end (), output.begin (), tolower);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string upperCase (const std::string& input)
{
  std::string output = input;
  std::transform (output.begin (), output.end (), output.begin (), toupper);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string ucFirst (const std::string& input)
{
  std::string output = input;

  if (output.length () > 0)
    output[0] = toupper (output[0]);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string str_replace (
  std::string &str,
  const std::string& search,
  const std::string& replacement)
{
  std::string::size_type pos = 0;
  while ((pos = str.find (search, pos)) != std::string::npos)
  {
    str.replace (pos, search.length (), replacement);
    pos += replacement.length ();
  }

  return str;
}

////////////////////////////////////////////////////////////////////////////////
std::string printable (const std::string& input)
{
  // Sanitize 'message'.
  std::string sanitized = input;
  std::string::size_type bad;
  while ((bad = sanitized.find ("\r")) != std::string::npos)
    sanitized.replace (bad, 1, "\\r");

  while ((bad = sanitized.find ("\n")) != std::string::npos)
    sanitized.replace (bad, 1, "\\n");

  while ((bad = sanitized.find ("\f")) != std::string::npos)
    sanitized.replace (bad, 1, "\\f");

  while ((bad = sanitized.find ("\t")) != std::string::npos)
    sanitized.replace (bad, 1, "\\t");

  while ((bad = sanitized.find ("\v")) != std::string::npos)
    sanitized.replace (bad, 1, "\\v");

  return sanitized;
}

////////////////////////////////////////////////////////////////////////////////
std::string printable (char input)
{
  // Sanitize 'message'.
  char stringized[2] = {0};
  stringized[0] = input;

  std::string sanitized = stringized;
  switch (input)
  {
  case '\r': sanitized = "\\r"; break;
  case '\n': sanitized = "\\n"; break;
  case '\f': sanitized = "\\f"; break;
  case '\t': sanitized = "\\t"; break;
  case '\v': sanitized = "\\v"; break;
  default:   sanitized = input; break;
  }

  return sanitized;
}

////////////////////////////////////////////////////////////////////////////////
bool digitsOnly (const std::string& input)
{
  for (size_t i = 0; i < input.length (); ++i)
    if (!isdigit (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool compare (
  const std::string& left,
  const std::string& right,
  bool sensitive /*= true*/)
{
  // Use strcasecmp if required.
  if (!sensitive)
    return strcasecmp (left.c_str (), right.c_str ()) == 0 ? true : false;

  // Otherwise, just use std::string::operator==.
  return left == right;
}

////////////////////////////////////////////////////////////////////////////////
bool closeEnough (
  const std::string& reference,
  const std::string& attempt,
  unsigned int minLength /* = 0 */)
{
  if (compare (reference, attempt, false))
    return true;

  if (attempt.length () < reference.length () &&
      attempt.length () >= minLength)
    return compare (reference.substr (0, attempt.length ()), attempt, false);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  bool sensitive /*= true*/)
{
  // Implement a sensitive find, which is really just a loop withing a loop,
  // comparing lower-case versions of each character in turn.
  if (!sensitive)
  {
    // Handle empty pattern.
    const char* p = pattern.c_str ();
    size_t len = pattern.length ();
    if (len == 0)
      return 0;

    // Evaluate these once, for performance reasons.
    const char* t = text.c_str ();
    const char* start = t;
    const char* end = start + text.size ();

    for (; t <= end - len; ++t)
    {
      int diff = 0;
      for (size_t i = 0; i < len; ++i)
        if ((diff = tolower (t[i]) - tolower (p[i])))
          break;

      // diff == 0 means there was no break from the loop, which only occurs
      // when a difference is detected.  Therefore, the loop terminated, and
      // diff is zero.
      if (diff == 0)
        return t - start;
    }

    return std::string::npos;
  }

  // Otherwise, just use std::string::find.
  return text.find (pattern);
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  std::string::size_type begin,
  bool sensitive /*= true*/)
{
  // Implement a sensitive find, which is really just a loop withing a loop,
  // comparing lower-case versions of each character in turn.
  if (!sensitive)
  {
    // Handle empty pattern.
    const char* p = pattern.c_str ();
    size_t len = pattern.length ();
    if (len == 0)
      return 0;

    // Handle bad begin.
    if (begin >= text.length ())
      return std::string::npos;

    // Evaluate these once, for performance reasons.
    const char* start = text.c_str ();
    const char* t = start + begin;
    const char* end = start + text.size ();

    for (; t <= end - len; ++t)
    {
      int diff = 0;
      for (size_t i = 0; i < len; ++i)
        if ((diff = tolower (t[i]) - tolower (p[i])))
          break;

      // diff == 0 means there was no break from the loop, which only occurs
      // when a difference is detected.  Therefore, the loop terminated, and
      // diff is zero.
      if (diff == 0)
        return t - start;
    }

    return std::string::npos;
  }

  // Otherwise, just use std::string::find.
  return text.find (pattern, begin);
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (char value)
{
  std::stringstream s;
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (int value)
{
  std::stringstream s;
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string formatHex (int value)
{
  std::stringstream s;
  s.setf (std::ios::hex, std::ios::basefield);
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (float value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = roundf (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = round (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value)
{
  std::stringstream s;
  s << std::fixed << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
static void replace_positional (
  std::string& fmt,
  const std::string& from,
  const std::string& to)
{
  std::string::size_type pos = 0;
  while ((pos = fmt.find (from, pos)) != std::string::npos)
  {
    fmt.replace (pos, from.length (), to);
    pos += to.length ();
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  const std::string& arg1)
{
  std::string output = fmt;
  replace_positional (output, "{1}", arg1);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  int arg1)
{
  std::string output = fmt;
  replace_positional (output, "{1}", format (arg1));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  const std::string& arg1,
  const std::string& arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", arg1);
  replace_positional (output, "{2}", arg2);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  const std::string& arg1,
  int arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", arg1);
  replace_positional (output, "{2}", format (arg2));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  const std::string& arg1,
  double arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", arg1);
  replace_positional (output, "{2}", trim (format (arg2, 6, 3)));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  int arg1,
  const std::string& arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", format (arg1));
  replace_positional (output, "{2}", arg2);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  int arg1,
  int arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", format (arg1));
  replace_positional (output, "{2}", format (arg2));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  int arg1,
  int arg2,
  int arg3)
{
  std::string output = fmt;
  replace_positional (output, "{1}", format (arg1));
  replace_positional (output, "{2}", format (arg2));
  replace_positional (output, "{3}", format (arg3));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  int arg1,
  double arg2)
{
  std::string output = fmt;
  replace_positional (output, "{1}", format (arg1));
  replace_positional (output, "{2}", trim (format (arg2, 6, 3)));
  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (
  const std::string& fmt,
  const std::string& arg1,
  const std::string& arg2,
  const std::string& arg3)
{
  std::string output = fmt;
  replace_positional (output, "{1}", arg1);
  replace_positional (output, "{2}", arg2);
  replace_positional (output, "{3}", arg3);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const int input, const int width)
{
  std::stringstream s;
  s << input;
  std::string output = s.str ();
  return output + std::string (width - output.length (), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustifyZero (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill ('0') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustify (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill (' ') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
