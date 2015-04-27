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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <Log.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
Log::Log ()
: _filename ("/tmp/default.log")
, _fh (NULL)
, _prior ("none")
, _repetition (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Log::~Log ()
{
  reset ();
}

////////////////////////////////////////////////////////////////////////////////
void Log::reset ()
{
  if (_fh)
  {
    fclose (_fh);
    _fh = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Log::setFile (const std::string& path)
{
  if (_filename != path)
  {
    reset ();
    _filename = path;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Log::write (const std::string& line, bool multiline /* = false */)
{
  // handle multilines by splitting them into single
  // lines and recursively calling Log::write
  if (multiline)
  {
    std::vector <std::string> lines;
    split (lines, line, "\n");
    std::vector <std::string>::iterator line;
    for (line = lines.begin (); line != lines.end (); ++line)
      write (line->c_str ());
    return;
  }

  if (line != "")
  {
    if (line == _prior)
    {
      ++_repetition;
      return;
    }

    _prior = line;

    if (_filename == "-")
    {
      timestamp ();

      if (_repetition)
      {
        printf ("%s (Repeated %d times)\n", _now, _repetition);
        _repetition = 0;
      }

      printf ("%s %s\n", _now, printable (line).c_str ());
      fflush (stdout);
    }
    else
    {
      if (!_fh)
        _fh = fopen (_filename.c_str (), "a");

      if (_fh)
      {
        timestamp ();

        if (_repetition)
        {
          fprintf (_fh, "%s (Repeated %d times)\n", _now, _repetition);
          _repetition = 0;
        }

        fprintf (_fh, "%s %s\n", _now, printable (line).c_str ());

        // To get around the auto file close of the daemonization process.
        fflush (_fh);
        fclose (_fh);
        _fh = NULL;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Log::format (const char* message, ...)
{
  // Crude and mostly ineffective check for buffer overrun.
  if (::strlen (message) >= 65536)
    throw std::string ("ERROR: Data exceeds 65,536 bytes.  Break data into smaller chunks.");

  char buffer[65536];
  va_list args;
  va_start (args, message);
  vsnprintf (buffer, 65536, message, args);
  va_end (args);

  std::string copy = buffer;
  this->write (copy);
}

////////////////////////////////////////////////////////////////////////////////
void Log::timestamp ()
{
  // Get time info, UTC.
  time_t current;
  time (&current);
  struct tm* t = gmtime (&current);

  // Generate timestamp.
  sprintf (_now, "%04d-%02d-%02d %02d:%02d:%02d",
                 t->tm_year + 1900,
                 t->tm_mon + 1,
                 t->tm_mday,
                 t->tm_hour,
                 t->tm_min,
                 t->tm_sec);
}

////////////////////////////////////////////////////////////////////////////////
