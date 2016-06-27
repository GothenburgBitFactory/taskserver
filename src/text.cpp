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
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <utf8.h>
#include <text.h>

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
// Damerau-Levenshtein
const int swap_penalty   = 1;
const int subst_penalty  = 1;
const int insert_penalty = 1;
const int delete_penalty = 1;

int damerau_levenshtein (const char* left, const char* right)
{
  const int len_left  = strlen (left);
  const int len_right = strlen (right);
  int grid[3][len_right + 1];
  int* line0 = &grid[0][0];
  int* line1 = &grid[1][0];
  int* line2 = &grid[2][0];

  for (int j = 0; j <= len_right; j++)
    line1[j] = j * insert_penalty;

  for (int i = 0; i < len_left; i++)
  {
    line2[0] = (i + 1) * delete_penalty;
    for (int j = 0; j < len_right; j++)
    {
      line2[j + 1] = line1[j] + (left[i] == right[j] ? 0 : subst_penalty);

      if (i > 0 &&
          j > 0 &&
          left[i - 1] == right[j] &&
          left[i] == right[j - 1] &&
          line2[j + 1] > line0[j - 1] + swap_penalty)
      {
        line2[j + 1] = line0[j - 1] + swap_penalty;
      }
      else if (line2[j + 1] > line1[j + 1] + delete_penalty)
      {
        line2[j + 1] = line1[j + 1] + delete_penalty;
      }
      else if (line2[j + 1] > line2[j] + insert_penalty)
      {
        line2[j + 1] = line2[j] + insert_penalty;
      }
    }

    int* temp = line0;
    line0 = line1;
    line1 = line2;
    line2 = temp;
  }

  return line1[len_right];
}

////////////////////////////////////////////////////////////////////////////////
