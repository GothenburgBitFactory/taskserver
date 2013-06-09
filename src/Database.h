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
  void authorize (const std::string&, const std::string&, const std::string&);
  bool redirect (const std::string&, Msg&);

  void organizations (std::vector <std::string>&);
  void groups (const std::string&, std::vector <std::string>&);
  void users (const std::string&, std::vector <std::string>&);

  void create_organization (const std::string&);
  void suspend_organization (const std::string&);
  void unsuspend_organization (const std::string&);
  void terminate_organization (const std::string&);
  void unterminate_organization (const std::string&);
  void delete_organization (const std::string&);

  void create_group (const std::string&, const std::string&);
  void delete_group (const std::string&, const std::string&);

  void create_user (const std::string&, const std::string&, std::string&);
  void suspend_user (const std::string&, const std::string&);
  void unsuspend_user (const std::string&, const std::string&);
  void terminate_user (const std::string&, const std::string&);
  void delete_user (const std::string&, const std::string&);
/*
  void add_user_to_group (const std::string&, const std::string&);
  void remove_user_from_group (const std::string&, const std::string&);
*/
  void grant (const std::string&, const std::string&, const std::string&, const std::string&);
  void revoke (const std::string&, const std::string&, const std::string&, const std::string&);
#endif

  std::string key_generate ();

#ifdef NONE_OF_THIS_WORKS
/*
  grant ();
  revoke ();
*/

  void read_rc (const std::string&, const std::string&, Config&);
  void read_data (const std::string&, const std::string&, std::vector <std::string>&);
  std::string add_data (const std::string&, const std::string&, const std::vector <std::string>&);

private:
  bool has_admin (const std::string&, const std::string&, const std::string&);
  bool has_org   (const std::string&, const std::string&, const std::string&);
  bool has_group (const std::string&, const std::string&, const std::string&, const std::string&);
  bool has_user  (const std::string&, const std::string&, const std::string&);
#endif

private:
  Config* _config;
  Directory _root;
  Log* _log;
};

#endif
////////////////////////////////////////////////////////////////////////////////
