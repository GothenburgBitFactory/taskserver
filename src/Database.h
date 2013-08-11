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

#ifndef INCLUDED_DATABASE
#define INCLUDED_DATABASE

#include <ConfigFile.h>
#include <Directory.h>
#include <Msg.h>
#include <Log.h>

class Database
{
public:
  Database (Config*);
  Database (const Database&);            // Copy constructor
  Database& operator= (const Database&); // Assignment operator
  ~Database ();                          // Destructor

  void setLog (Log*);

  // These throw on failure.
  bool authenticate (const Msg&, Msg&);
#ifdef NONE_OF_THIS_WORKS
  bool redirect (const std::string&, Msg&);
#endif

  bool add_org (const std::string&);
  bool add_group (const std::string&, const std::string&);
  bool remove_org (const std::string&);

  std::string key_generate ();

public:
  Config* _config;

private:
  Log* _log;
};

#endif
////////////////////////////////////////////////////////////////////////////////
