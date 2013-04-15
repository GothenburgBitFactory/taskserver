////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2010 - 2013, Göteborg Bit Factory.
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

#ifndef INCLUDED_TASK
#define INCLUDED_TASK

#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <cmake.h>

class Task : public std::map <std::string, std::string>
{
public:
  Task ();                       // Default constructor
  Task (const Task&);            // Copy constructor
  Task& operator= (const Task&); // Assignment operator
  bool operator== (const Task&); // Comparison operator
  Task (const std::string&);     // Parse
  ~Task ();                      // Destructor

  void parse (const std::string&);
  std::string composeF4 () const;

  // Status values.
  enum status {pending, completed, deleted, recurring, waiting};

  // Public data.
  int id;

  // Series of helper functions.
  static status textToStatus (const std::string&);
  static std::string statusToText (status);

  void setModified ();

  bool has (const std::string&) const;
  std::vector <std::string> all ();
  const std::string get (const std::string&) const;
  time_t get_date (const std::string&) const;
  void set (const std::string&, const std::string&);
  void set (const std::string&, int);                                           
  void remove (const std::string&);

  status getStatus () const;
  void setStatus (status);

  void validate ();
};

#endif
////////////////////////////////////////////////////////////////////////////////
