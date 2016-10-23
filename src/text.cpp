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
#include <text.h>
#include <string.h>

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
