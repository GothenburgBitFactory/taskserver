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
#include <sstream>
#include <algorithm>
#include <utf8.h>
#include <Path.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <cmake.h>
#include <Database.h>

////////////////////////////////////////////////////////////////////////////////
Database::Database (Config& c)
: _config (c)
{
  std::string data_root = c.get ("root");
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

#ifdef NONE_OF_THIS_WORKS
////////////////////////////////////////////////////////////////////////////////
// Authentication is when the org/user/key data exists/matches that on the
// server, in the absence of account suspension or termination.
void Database::authenticate (
  const std::string& org,
  const std::string& user,
  const std::string& key)
{
  // Verify non-existence of <root>/orgs/<org>/terminated
  File org_terminated (_root._data + "/orgs/" + org + "/terminated");
  if (org_terminated.exists ())
  {
    if (_log)
      _log->write ("Database::authenticate found " + org_terminated._data);
    throw 432;
  }

  // Verify non-existence of <root>/orgs/<org>/suspended
  File org_suspended (_root._data + "/orgs/" + org + "/suspended");
  if (org_suspended.exists ())
  {
    if (_log)
      _log->write ("Database::authenticate found " + org_suspended._data);
    throw 431;
  }

  // Verify existence of <root>/orgs/<org>/users/<user>
  Path user_dir (_root._data + "/orgs/" + org + "/users/" + user);
  if (!user_dir.exists ())
  {
    if (_log)
      _log->write ("Database::authenticate missing " + user_dir._data);
    throw 430;
  }

  // Verify non-existence of <root>/orgs/<org>/users/<user>/terminated
  File user_terminated (_root._data + "/orgs/" + org + "/users/" + user + "/terminated");
  if (user_terminated.exists ())
  {
    if (_log)
      _log->write ("Database::authenticate found " + user_terminated._data);
    throw 432;
  }

  // Verify non-existence of <root>/orgs/<org>/users/<user>/suspended
  File user_suspended (_root._data + "/orgs/" + org + "/users/" + user + "/suspended");
  if (user_suspended.exists ())
  {
    if (_log)
      _log->write ("Database::authenticate found " + user_suspended._data);
    throw 431;
  }

  // Match <key> against <root>/orgs/<org>/users/<user>/rc:<key>
  Config user_rc (_root._data + "/orgs/" + org + "/users/" + user + "/rc");
  if (user_rc.get ("key") != key)
  {
    if (_log)
      _log->write ("Database::authenticate key mismatch in " + _root._data + "/orgs/" + org + "/users/" + user + "/rc");
    throw 430;
  }

  // No failures means successful authentication.
  if (_log)
    _log->write ("Authenticated '" + org + "/" + user + "'");
}

////////////////////////////////////////////////////////////////////////////////
// Authorization is when the specified token is found at the org, user or group
// level.
//
// Tokens may exist in ...
void Database::authorize (
  const std::string& org,
  const std::string& user,
  const std::string& token)
{
  // Anyone can synch.
  if (token == "synch")
    return;

  // Determine whether user has root-level privileges.
  bool root = has_admin (org, user, "root");

  // Root-level privileges.
  if (token == "org-list"        ||
      token == "org-create"      ||
      token == "org-delete"      ||
      token == "org-terminate"   ||
      token == "org-suspend"     ||
      token == "org-unsuspend"   ||
      token == "org-redirect"    ||
      token == "stats")
  {
    if (_log)
      _log->write ("Authorized '" + org + "/" + user + "' for '" + token + "'");

    if (root || has_admin (org, user, token))
      return;
  }

  // Organization-level privileges.
  if (token == "group-create"   ||
      token == "group-delete"   ||
      token == "group-list"     ||
      token == "user-list"      ||
      token == "user-create"    ||
      token == "user-delete"    ||
      token == "user-terminate" ||
      token == "user-suspend"   ||
      token == "user-unsuspend")
  {
    if (_log)
      _log->write ("Authorized '" + org + "/" + user + "' for '" + token + "'");

    if (root || has_org (org, user, token))
      return;
  }

  // TODO Unhandled ticket: "command"

  // Default response is no authorization.
  if (_log)
    _log->write ("Failed authorization '" + org + "/" + user + "' for '" + token + "'");

  throw 430; // Access denied.
}

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

////////////////////////////////////////////////////////////////////////////////
void Database::organizations (std::vector <std::string>& all)
{
  all.clear ();
  Directory d (_root._data + "/orgs");
  all = d.list ();

  std::vector <std::string>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    *it = Path (*it).name ();
}

////////////////////////////////////////////////////////////////////////////////
void Database::groups (const std::string& org, std::vector <std::string>& all)
{
  all.clear ();
  Directory d (_root._data + "/orgs/" + org + "/groups");
  all = d.list ();

  std::vector <std::string>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    *it = Path (*it).name ();
}

////////////////////////////////////////////////////////////////////////////////
void Database::users (const std::string& org, std::vector <std::string>& all)
{
  all.clear ();
  Directory d (_root._data + "/orgs/" + org + "/users");
  all = d.list ();

  std::vector <std::string>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    *it = Path (*it).name ();
}

////////////////////////////////////////////////////////////////////////////////
// Create organization directory and empty groups and users directories.
void Database::create_organization (const std::string& org)
{
  if (! Directory (_root._data + "/orgs/" + org).create () ||
      ! Directory (_root._data + "/orgs/" + org + "/groups").create () ||
      ! Directory (_root._data + "/orgs/" + org + "/users").create ())
    throw std::string ("Failed to create organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::suspend_organization (const std::string& org)
{
  File flag (_root._data + "/orgs/" + org + "/suspended");
  if (! flag.create ())
    throw std::string ("Failed to suspend organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::unsuspend_organization (const std::string& org)
{
  File flag (_root._data + "/orgs/" + org + "/suspended");
  if (! flag.remove ())
    throw std::string ("Failed to unsuspend organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::terminate_organization (const std::string& org)
{
  File flag (_root._data + "/orgs/" + org + "/terminated");
  if (! flag.create ())
    throw std::string ("Failed to terminate organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::unterminate_organization (const std::string& org)
{
  File flag (_root._data + "/orgs/" + org + "/terminated");
  if (! flag.remove ())
    throw std::string ("Failed to unterminate organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::delete_organization (const std::string& org)
{
  if (! Directory (_root._data + "/orgs/" + org).remove ())
    throw std::string ("Failed to delete organization '") + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::create_group (const std::string& org, const std::string& group)
{
  if (! Directory (_root._data + "/orgs/" + org + "/groups").create ())
    throw std::string ("Failed to create group '") + group + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::delete_group (const std::string& org, const std::string& group)
{
  if (! Directory (_root._data + "/orgs/" + org + "/groups/" + group).remove ())
    throw std::string ("Failed to delete group '") + group + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::create_user (
  const std::string& org,
  const std::string& user,
  std::string& key)
{
  std::string path = _root._data + "/orgs/" + org + "/users/" + user;
  File rc (path + "/rc");

  if (! Directory (path).create () ||
      ! rc.create () ||
      ! File (path + "/tx.data").create ())
    throw std::string ("Failed to create user '") + user + "' for organization '" + org + "'";

  // Generate a user key and store it in the rc file. 
  key = key_generate ();
  rc.append (std::string ("key=") + key + "\n");
}

////////////////////////////////////////////////////////////////////////////////
void Database::suspend_user (const std::string& org, const std::string& user)
{
  File flag (_root._data + "/orgs/" + org + "/users/" + user + "/suspended");
  if (! flag.create ())
    throw std::string ("Failed to suspend user '") + user + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::unsuspend_user (const std::string& org, const std::string& user)
{
  File flag (_root._data + "/orgs/" + org + "/users/" + user + "/suspended");
  if (! flag.remove ())
    throw std::string ("Failed to unsuspend user '") + user + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::terminate_user (const std::string& org, const std::string& user)
{
  File flag (_root._data + "/orgs/" + org + "/users/" + user + "/terminated");
  if (! flag.create ())
    throw std::string ("Failed to terminate user '") + user + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
void Database::delete_user (const std::string& org, const std::string& user)
{
  if (! Directory (_root._data + "/orgs/" + org + "/users/" + user).remove ())
    throw std::string ("Failed to delete user '") + user + "' for organization '" + org + "'";
}

////////////////////////////////////////////////////////////////////////////////
/*
       permissions.Individual/bob=group-create,group-delete

       Context  Token              Meaning
       -------  -----------------  --------------------------------------------
       root                        Unrestricted

       admin    stats              Can examine server stats
       admin    org-create         Can create an organization
       admin    org-delete         Can delete an organization
       admin    org-list           Can examine list of organizations
       admin    org-terminate      Can terminate an organization (irreversible)
       admin    org-suspend        Can suspend an organization
       admin    org-unsuspend      Can unsuspend an organization
       admin    org-redirect       Can relocate an organization

       org      group-create       Can create a group
       org      group-delete       Can delete a group
       org      group-list         Can examine list of groups
       org      user-create        Can create a user
       org      user-delete        Can delete a user
       org      user-list          Can examine list of users
       org      user-terminate     Can terminate a user (irreversible)
       org      user-suspend       Can suspend a user
       org      user-unsuspend     Can unsuspend a user

       group    user-add-group     Can add a user to a group
       group    user-remove-group  Can remove a user from a group

   Root and Admin privileges are only stored in the /rc file.
   Org privileges are only stored in the /orgs/<org>/rc file.
   Group privileges are only stored in the /orgs/<org>/groups/<group>/rc file.
   User privileges (there are none yet specified) would only be stored in the
   /orgs/<org>/users/<user>/rc file.
*/
void Database::grant (
  const std::string& org,
  const std::string& user,
  const std::string& group,
  const std::string& privilege)
{
  // Is user authorized?
  // Determine which file to update
}

////////////////////////////////////////////////////////////////////////////////
void Database::revoke (
  const std::string& org,
  const std::string& user,
  const std::string& group,
  const std::string& privilege)
{
}
#endif

////////////////////////////////////////////////////////////////////////////////
std::string Database::key_generate ()
{
  return uuid ();
}

#ifdef NONE_OF_THIS_WORKS
////////////////////////////////////////////////////////////////////////////////
void Database::read_rc (
  const std::string& org,
  const std::string& user,
  Config& config)
{
  File cfg (_root._data + "/orgs/" + org + "/users/" + user + "/rc");
  config.load (cfg._data);
}

////////////////////////////////////////////////////////////////////////////////
void Database::read_data (
  const std::string& org,
  const std::string& user,
  std::vector <std::string>& data)
{
  File tx (_root._data + "/orgs/" + org + "/users/" + user + "/tx.data");
  tx.read (data);
}

////////////////////////////////////////////////////////////////////////////////
std::string Database::add_data (
  const std::string& org,
  const std::string& user,
  const std::vector <std::string>& data)
{
  // Write the data.
  File tx (_root._data + "/orgs/" + org + "/users/" + user + "/tx.data");
  tx.append (data);

  // Generate a uuid and write it.
  std::string id = uuid ();
  tx.append (id + "\n");

  // Return new id.
  return id;
}

////////////////////////////////////////////////////////////////////////////////
bool Database::has_admin (
  const std::string& org,
  const std::string& user,
  const std::string& token)
{
  // Determine user id.
  std::string id = org + "/" + user;

  // Look for the token.
  std::vector <std::string> tokens;
  split (tokens, _config.get ("permissions." + id), ',');

  return std::find (tokens.begin (), tokens.end (), token) != tokens.end ()
    ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Database::has_org (
  const std::string& org,
  const std::string& user,
  const std::string& token)
{
  // Load the correct rc file.
  File cfg (_root._data + "/orgs/" + org + "/rc");
  Config rc;
  rc.load (cfg._data);

  // Determine user id.
  std::string id = org + "/" + user;

  // Look for the token.
  std::vector <std::string> tokens;
  split (tokens, rc.get ("permissions." + id), ',');

  return std::find (tokens.begin (), tokens.end (), token) != tokens.end ()
    ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Database::has_group (
  const std::string& org,
  const std::string& user,
  const std::string& group,
  const std::string& token)
{
  // Load the correct rc file.
  File cfg (_root._data + "/orgs/" + org + "/groups/" + group + "/rc");
  Config rc;
  rc.load (cfg._data);

  // Determine user id.
  std::string id = org + "/" + user;

  // Look for the token.
  std::vector <std::string> tokens;
  split (tokens, rc.get ("permissions." + id), ',');

  return std::find (tokens.begin (), tokens.end (), token) != tokens.end ()
    ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Database::has_user (
  const std::string& org,
  const std::string& user,
  const std::string& token)
{
  // Load the correct rc file.
  File cfg (_root._data + "/orgs/" + org + "/users/" + user + "/rc");
  Config rc;
  rc.load (cfg._data);

  // Determine user id.
  std::string id = org + "/" + user;

  // Look for the token.
  std::vector <std::string> tokens;
  split (tokens, rc.get ("permissions." + id), ',');

  return std::find (tokens.begin (), tokens.end (), token) != tokens.end ()
    ? true : false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
