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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <text.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
// Uses std::getline, because std::cin eats leading whitespace, and that means
// that if a newline is entered, std::cin eats it and never returns from the
// "std::cin >> answer;" line, but it does display the newline.  This way, with
// std::getline, the newline can be detected, and the prompt re-written.
bool confirm (const std::string& question)
{
  std::vector <std::string> options;
  options.push_back ("yes");
  options.push_back ("no");

  std::string answer;
  std::vector <std::string> matches;

  do
  {
    std::cout << question << " (yes/no) ";

    std::getline (std::cin, answer);
    answer = std::cin.eof() ? "no" : lowerCase (trim (answer));

    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (matches.size () != 1);

  return matches[0] == "yes" ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in bytes to a more readable format.
std::string formatBytes (size_t bytes)
{
  char formatted[24];

       if (bytes >=  995000000) sprintf (formatted, "%.1f GiB", bytes / 1000000000.0);
  else if (bytes >=     995000) sprintf (formatted, "%.1f MiB", bytes /    1000000.0);
  else if (bytes >=        995) sprintf (formatted, "%.1f KiB", bytes /       1000.0);
  else                          sprintf (formatted, "%d B",     (int)bytes);

  return commify (formatted);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
std::string formatTime (time_t seconds)
{
  char formatted[24];
  float days = (float) seconds / 86400.0;

       if (seconds >= 86400 * 365) sprintf (formatted, "%.1f y", (days / 365.0));
  else if (seconds >= 86400 * 84)  sprintf (formatted, "%1d mo", (int) (days / 30));
  else if (seconds >= 86400 * 13)  sprintf (formatted, "%d wk",  (int) (float) (days / 7.0));
  else if (seconds >= 86400)       sprintf (formatted, "%d d",   (int) days);
  else if (seconds >= 3600)        sprintf (formatted, "%d h",   (int) (seconds / 3600));
  else if (seconds >= 60)          sprintf (formatted, "%d m",   (int) (seconds / 60));
  else if (seconds >= 1)           sprintf (formatted, "%d s",   (int) seconds);
  else                             strcpy (formatted, "-");

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
int autoComplete (
  const std::string& partial,
  const std::vector<std::string>& list,
  std::vector<std::string>& matches,
  int minimum/* = 1*/)
{
  matches.clear ();

  // Handle trivial case. 
  unsigned int length = partial.length ();
  if (length)
  {
    std::vector <std::string>::const_iterator item;
    for (item = list.begin (); item != list.end (); ++item)
    {
      // An exact match is a special case.  Assume there is only one exact match
      // and return immediately.
      if (partial == *item)
      {
        matches.clear ();
        matches.push_back (*item);
        return 1;
      }

      // Maintain a list of partial matches.
      else if (length >= (unsigned) minimum &&
               length <= item->length ()    &&
               partial == item->substr (0, length))
        matches.push_back (*item);
    }
  }

  return matches.size ();
}

// Handle the generation of UUIDs on FreeBSD in a separate implementation
// of the uuid () function, since the API is quite different from Linux's.
// Also, uuid_unparse_lower is not needed on FreeBSD, because the string
// representation is always lowercase anyway.
// For the implementation details, refer to
// http://svnweb.freebsd.org/base/head/sys/kern/kern_uuid.c
#ifdef FREEBSD
const std::string uuid ()
{
  uuid_t id;
  uint32_t status;
  char *buffer (0);
  uuid_create (&id, &status);
  uuid_to_string (&id, &buffer, &status);

  std::string res (buffer);
  free (buffer);

  return res;
}
#else

////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_UUID_UNPARSE_LOWER
// Older versions of libuuid don't have uuid_unparse_lower(), only uuid_unparse()
void uuid_unparse_lower (uuid_t uu, char *out)
{
    uuid_unparse (uu, out);
    // Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
    // a-z by bitwise or with 0x20, and the others already have this bit set
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}
#endif

const std::string uuid ()
{
  uuid_t id;
  uuid_generate (id);
  char buffer[100] = {0};
  uuid_unparse_lower (id, buffer);

  // Bug found by Steven de Brouwer.
  buffer[36] = '\0';

  return std::string (buffer);
}
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   [  -> &open;
//   ]  -> &close;
const std::string encode (const std::string& value)
{
  std::string modified = value;

  str_replace (modified, "[",  "&open;");
  str_replace (modified, "]",  "&close;");

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   "  <- &dquot;
//   '  <- &squot; or &quot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
//   :  <- &colon;
const std::string decode (const std::string& value)
{
  if (value.find ('&') != std::string::npos)
  {
    std::string modified = value;

    // Supported encodings.
    str_replace (modified, "&open;",  "[");
    str_replace (modified, "&close;", "]");

    // Support for deprecated encodings.  These cannot be removed or old files
    // will not be parsable.  Not just old files - completed.data can contain
    // tasks formatted/encoded using these.
    str_replace (modified, "&dquot;", "\"");
    str_replace (modified, "&quot;",  "'");
    str_replace (modified, "&squot;", "'");  // Deprecated 2.0
    str_replace (modified, "&comma;", ",");  // Deprecated 2.0
    str_replace (modified, "&colon;", ":");  // Deprecated 2.0

    return modified;
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Escapes any unescaped character of type c within the given string
// e.g. ' ' -> '\ '
const std::string escape (const std::string& value, char c)
{
  std::string modified = value;
  char tmp[2] = {c, '\0'};
  std::string search  = tmp;
  std::string replace = "\\" + search;

  std::string::size_type pos = modified.find (search);
  while (pos != std::string::npos) {
    if ( modified[pos-1] != '\\' )
      modified.replace (pos, 1, replace);

    pos = modified.find (search, pos+1);
  }

  return modified;
}


////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_TIMEGM
time_t timegm (struct tm *tm)
{
  time_t ret;
  char *tz;
  tz = getenv ("TZ");
  setenv ("TZ", "UTC", 1);
  tzset ();
  ret = mktime (tm);
  if (tz)
    setenv ("TZ", tz, 1);
  else
    unsetenv ("TZ");
  tzset ();
  return ret;
}
#endif

////////////////////////////////////////////////////////////////////////////////
