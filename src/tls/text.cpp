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

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <text.h>

static void replace_positional (std::string&, const std::string&, const std::string&);

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
