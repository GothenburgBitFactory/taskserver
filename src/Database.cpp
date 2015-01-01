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
#include <sstream>
#include <algorithm>
#include <utf8.h>
#include <Path.h>
#include <Directory.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <taskd.h>
#include <Database.h>

////////////////////////////////////////////////////////////////////////////////
Database::Database (Config* config)
: _config (config)
, _log (NULL)
{
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
  Directory org_dir (_config->get ("root") + "/orgs/" + org);
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
  File org_suspended (_config->get ("root") + "/orgs/" + org + "/suspended");
  if (org_suspended.exists ())
  {
    if (_log)
      _log->format ("INFO Auth failure: org '%s' suspended",
                    org.c_str ());

    response.set ("code", 431);
    response.set ("status", taskd_error (431));
    return false;
  }

  // Verify existence of <root>/orgs/<org>/users/<key>
  Directory user_dir (_config->get ("root") + "/orgs/" + org + "/users/" + key);
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

  // Verify non-existence of <root>/orgs/<org>/users/<key>/suspended
  File user_suspended (_config->get ("root") + "/orgs/" + org + "/users/" + key + "/suspended");
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

  // Match <user> against <root>/orgs/<org>/users/<key>/rc:<user>
  Config user_rc (_config->get ("root") + "/orgs/" + org + "/users/" + key + "/config");
  if (!user.empty () && user_rc.get ("user") != user)
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

////////////////////////////////////////////////////////////////////////////////
// if <root>/orgs/<org>/redirect exists, read it and send contents as a 301.
bool Database::redirect (const std::string& org, Msg& response)
{
  File redirect (_config->get ("root"));
  redirect += "orgs";
  redirect += org;
  redirect += "redirect";

  if (redirect.exists ())
  {
    std::string server;
    redirect.read (server);
    response.set ("code", 301);
    response.set ("info", trim (server, " \n"));

    if (_log)
      _log->format ("INFO Redirecting org '%s' to '%s'",
                    org.c_str (),
                    response.get ("info").c_str ());

    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////
bool Database::add_org (const std::string& org)
{
  Directory new_org (_config->get ("root"));
  new_org += "orgs";
  new_org += org;

  Directory groups (new_org);
  groups += "groups";

  Directory users (new_org);
  users += "users";

  return new_org.create (0700) &&
         groups.create (0700) &&
         users.create (0700);
}

////////////////////////////////////////////////////////////////////////////////
bool Database::add_group (
  const std::string& org,
  const std::string& group)
{
  Directory new_group (_config->get ("root"));
  new_group += "orgs";
  new_group += org;
  new_group += "groups";
  new_group += group;

  return new_group.create (0700);
}

////////////////////////////////////////////////////////////////////////////////
bool Database::add_user (
  const std::string& org,
  const std::string& user)
{
  // Generate new KEY
  std::string key = key_generate ();

  Directory new_user (_config->get ("root"));
  new_user += "orgs";
  new_user += org;
  new_user += "users";
  new_user += key;

  if (new_user.create (0700))
  {
    // Store USER in <new_user>/config
    File conf_file (new_user._data + "/config");
    conf_file.create (0600);

    Config conf (conf_file._data);
    conf.set ("user", user);
    conf.save ();

    // User will need this key.
    std::cout << "New user key: " << key << "\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Database::remove_org (const std::string& org)
{
  Directory org_dir (_config->get ("root"));
  org_dir += "orgs";
  org_dir += org;

  // TODO Remove users?
  // TODO Remove groups?
  // TODO Revoke user group membership.

  return org_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
bool Database::remove_group (
  const std::string& org,
  const std::string& group)
{
  Directory group_dir (_config->get ("root"));
  group_dir += "orgs";
  group_dir += org;
  group_dir += "groups";
  group_dir += group;

  // TODO Revoke user group membership.

  return group_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
bool Database::remove_user (
  const std::string& org,
  const std::string& key)
{
  Directory user_dir (_config->get ("root"));
  user_dir += "orgs";
  user_dir += org;
  user_dir += "users";
  user_dir += key;

  // TODO Revoke group memberships.

  return user_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
bool Database::suspend (const Directory& node)
{
  File semaphore (node._data + "/suspended");
  return semaphore.create (0600);
}

////////////////////////////////////////////////////////////////////////////////
bool Database::resume (const Directory& node)
{
  File semaphore (node._data + "/suspended");
  return semaphore.remove ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Database::key_generate ()
{
  return uuid ();
}

////////////////////////////////////////////////////////////////////////////////
