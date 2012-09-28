////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2010 - 2012, Göteborg Bit Factory.
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

#include <iostream>
#include <stdio.h>
#include <string.h>
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
  int minimum/* = 2*/)
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

////////////////////////////////////////////////////////////////////////////////

