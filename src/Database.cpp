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

#include <cmake.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <utf8.h>
#include <Path.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <taskd.h>
#include <Database.h>

////////////////////////////////////////////////////////////////////////////////
Database::Database (Config* config)
: _config (config)
, _root ()
, _log (NULL)
{
/*
  std::string data_root = _config->get ("root");
  if (data_root == "")
    throw std::string ("root is not set");

  // Set up and validate root.
  _root = Directory (data_root);
  if (!_root.exists ())
    throw std::string ("root '") + data_root + "' does not exist";

  if (!_root.is_directory ())
    throw std::string ("root '") + data_root + "' is not a directory";

  if (!_root.readable ())
    throw std::string ("root '") + data_root + "' is not readable";

  if (!_root.readable ())
    throw std::string ("root '") + data_root + "' is not writable";
*/
}

////////////////////////////////////////////////////////////////////////////////
Database::~Database ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Database::setLog (Log* l)
{
  _log = l;
}

////////////////////////////////////////////////////////////////////////////////
// Authentication is when the org/user/key data exists/matches that on the
// server, in the absence of org/user account suspension.
//
// If authentication fails, fills in response code and status.
// Note: information regarding valid/invalid org/user is not revealed.
bool Database::authenticate (
  const Msg& request,
  Msg& response)
{
  std::string org  = request.get ("org");
  std::string user = request.get ("user");
  std::string key  = request.get ("key");

  // Verify existence of <root>/orgs/<org>
  Directory org_dir (_root._data + "/orgs/" + org);
  if (! org_dir.exists ())
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' unknown",
                    org.c_str ());

    response.set ("code", 430);
    response.set ("status", taskd_error (430));
    return false;
  }

  // Verify non-existence of <root>/orgs/<org>/suspended
  File org_suspended (_root._data + "/orgs/" + org + "/suspended");
  if (org_suspended.exists ())
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' suspended",
                    org.c_str ());

    response.set ("code", 431);
    response.set ("status", taskd_error (431));
    return false;
  }

  // Verify existence of <root>/orgs/<org>/users/<user>
  Directory user_dir (_root._data + "/orgs/" + org + "/users/" + user);
  if (! user_dir.exists ())
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' user '%s' unknown",
                    org.c_str (),
                    user.c_str ());

    response.set ("code", 430);
    response.set ("status", taskd_error (430));
    return false;
  }

  // Verify non-existence of <root>/orgs/<org>/user/<user>/suspended
  File user_suspended (_root._data + "/orgs/" + org + "/user/" + user + "/suspended");
  if (user_suspended.exists ())
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' user '%s' suspended",
                    org.c_str (),
                    user.c_str ());

    response.set ("code", 431);
    response.set ("status", taskd_error (431));
    return false;
  }

  // Match <key> against <root>/orgs/<org>/users/<user>/rc:<key>
  Config user_rc (_root._data + "/orgs/" + org + "/users/" + user + "/config");
  if (user_rc.get ("key") != key)
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' user '%s' bad key",
                    org.c_str (),
                    user.c_str ());

    response.set ("code", 430);
    response.set ("status", taskd_error (430));
    return false;
  }

  // All checks succeed, user is authenticated.
  return true;
}

#ifdef NONE_OF_THIS_WORKS
////////////////////////////////////////////////////////////////////////////////
// if <root>/rc contains redirect.<org>=<server>:<port>, issue a 301.
bool Database::redirect (const std::string& org, Msg& response)
{
  Config rc (_root._data + "/rc");

  std::string server = rc.get ("redirect." + org);
  if (server != "")
  {
    if (_log)
      _log->write ("Database::redirect: " + org + " -> " + server);

    response.addStatus (301, server, "");
    return true;
  }

  return false;
}

#endif

////////////////////////////////////////////////////////////////////////////////
std::string Database::key_generate ()
{
  return uuid ();
}

////////////////////////////////////////////////////////////////////////////////
