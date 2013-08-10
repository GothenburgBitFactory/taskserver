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
#include <stdlib.h>
#include <ConfigFile.h>
#include <taskd.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
static bool add_org (
  const Directory& root,
  const std::string& org)
{
  Directory new_org (root);
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
static bool add_group (
  const Directory& root,
  const std::string& org,
  const std::string& group)
{
  Directory new_group (root);
  new_group += "orgs";
  new_group += org;
  new_group += "groups";
  new_group += group;

  return new_group.create (0700);
}

////////////////////////////////////////////////////////////////////////////////
static bool add_user (
  const Directory& root,
  const std::string& org,
  const std::string& user)
{
  Directory new_user (root);
  new_user += "orgs";
  new_user += org;
  new_user += "users";
  new_user += user;

  if (new_user.create (0700))
  {
    // Generate new KEY
    std::string key = taskd_generate_key ();

    // Store KEY in <new_user>/config
    File conf_file (new_user._data + "/config");
    conf_file.create (0600);

    Config conf (conf_file._data);
    conf.set ("key", key);
    conf.save ();

    // User will need this key.
    std::cout << "New user key: " << key << "\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool remove_org (
  const Directory& root,
  const std::string& org)
{
  Directory org_dir (root);
  org_dir += "orgs";
  org_dir += org;

  // TODO Remove users?
  // TODO Remove groups?
  // TODO Revoke user group membership.

  return org_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
static bool remove_group (
  const Directory& root,
  const std::string& org,
  const std::string& group)
{
  Directory group_dir (root);
  group_dir += "orgs";
  group_dir += org;
  group_dir += "groups";
  group_dir += group;

  // TODO Revoke user group membership.

  return group_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
static bool remove_user (
  const Directory& root,
  const std::string& org,
  const std::string& user)
{
  Directory user_dir (root);
  user_dir += "orgs";
  user_dir += org;
  user_dir += "users";
  user_dir += user;

  // TODO Revoke group memberships.

  return user_dir.remove ();
}

////////////////////////////////////////////////////////////////////////////////
static bool suspend_node (const Directory& node)
{
  File semaphore (node._data + "/suspended");
  return semaphore.create (0600);
}

////////////////////////////////////////////////////////////////////////////////
static bool resume_node (const Directory& node)
{
  File semaphore (node._data + "/suspended");
  return semaphore.remove ();
}

////////////////////////////////////////////////////////////////////////////////
// taskd add org   <org>
// taskd add group <org> <group>
// taskd add user  <org> <user>
void command_add (Database& db, const std::vector <std::string>& args)
{
  bool verbose = db._config->getBoolean ("verbose");

  // Verify that root exists.
  std::string root = db._config->get ("root");
  if (root == "")
    throw std::string ("ERROR: The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  if (args.size () < 1)
    throw std::string ("ERROR: Subcommand not specified - expected 'org', 'group' or 'user'.");

  // Create an organization.
  //   org <org>
  if (closeEnough ("org", args[1], 3))
  {
    if (args.size () < 3)
      throw std::string ("Usage: taskd add [options] org <org>");

    for (unsigned int i = 2; i < args.size (); ++i)
    {
      if (taskd_is_org (root_dir, args[i]))
        throw std::string ("ERROR: Organization '") + args[i] + "' already exists.";

      if (add_org (root_dir, args[i]))
      {
        if (verbose)
          std::cout << "Created organization '" << args[i] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to create organization '") + args[i] + "'.";
    }
  }

  // Create a group.
  //   group <org> <group>
  else if (closeEnough ("group", args[1], 3))
  {
    if (args.size () < 4)
      throw std::string ("Usage: taskd add [options] group <org> <group>");

    if (! taskd_is_org (root_dir, args[2]))
      throw std::string ("ERROR: Organization '") + args[1] + "' does not exist.";

    for (unsigned int i = 3; i < args.size (); ++i)
    {
      if (taskd_is_group (root_dir, args[2], args[i]))
        throw std::string ("ERROR: Group '") + args[i] + "' already exists.";

      if (add_group (root_dir, args[2], args[i]))
      {
        if (verbose)
          std::cout << "Created group '" << args[i] << "' for organization '" << args[2] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to create group '") + args[i] + "'.";
    }
  }

  // Create a user.
  //   user <org> <user>
  else if (closeEnough ("user", args[1], 3))
  {
    if (args.size () < 4)
      throw std::string ("Usage: taskd add [options] user <org> <user>");

    if (! taskd_is_org (root_dir, args[2]))
      throw std::string ("ERROR: Organization '") + args[1] + "' does not exist.";

    for (unsigned int i = 3; i < args.size (); ++i)
    {
      if (taskd_is_user (root_dir, args[2], args[i]))
        throw std::string ("ERROR: User '") + args[i] + "' already exists.";

      if (add_user (root_dir, args[2], args[i]))
      {
        if (verbose)
          std::cout << "Created user '" << args[i] << "' for organization '" << args[2] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to create user '") + args[i] + "'.";
    }
  }

  else
    throw std::string ("ERROR: Unrecognized argument '") + args[1] + "'";
}

////////////////////////////////////////////////////////////////////////////////
// taskd remove org   <org>
// taskd remove group <org> <group>
// taskd remove user  <org> <user>
int command_remove (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  std::string root = "";

  std::vector <std::string> positional;
  bool verbose = true;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--data",   *i, 3))  root = *(++i);
    else if (closeEnough ("--quiet",  *i, 3))  verbose = false;
    else if (closeEnough ("--debug",  *i, 3))  ; // TODO Is this necessary?
    else if (taskd_applyOverride (config, *i)) ;
    else
      positional.push_back (*i);
  }

  if (root == "")
  {
    char* root_env = getenv ("TASKDDATA");
    if (root_env)
      root = root_env;
  }

  // Verify that root exists.
  if (root == "")
    throw std::string ("ERROR: The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  if (positional.size () < 1)
    throw std::string ("ERROR: Subcommand not specified - expected 'org', 'group' or 'user'.");

  // Remove an organization.
  //   org <org>
  if (closeEnough ("org", positional[0], 3))
  {
    if (positional.size () < 2)
      throw std::string ("Usage: taskd remove [options] org <org>");

    for (unsigned int i = 1; i < positional.size (); ++i)
    {
      if (! taskd_is_org (root_dir, positional[i]))
        throw std::string ("ERROR: Organization '") + positional[i] + "' does not exist.";

      if (remove_org (root_dir, positional[i]))
      {
        if (verbose)
          std::cout << "Removed organization '" << positional[i] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to remove organization '") + positional[i] + "'.";
    }
  }

  // Remove a group.
  //   group <org> <group>
  else if (closeEnough ("group", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd remove [options] group <org> <group>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_group (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: Group '") + positional[i] + "' does not exist.";

      if (remove_group (root_dir, positional[1], positional[i]))
      {
        if (verbose)
          std::cout << "Removed group '" << positional[i] << "' from organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to remove group '") + positional[i] + "'.";
    }
  }

  // Remove a user.
  //   user <org> <user>
  else if (closeEnough ("user", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd remove [options] user <org> <user>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_user (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: User '") + positional[i] + "' does not  exists.";

      if (remove_user (root_dir, positional[1], positional[i]))
      {
        if (verbose)
          std::cout << "Removed user '" << positional[i] << "' from organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to remove user '") + positional[i] + "'.";
    }
  }

  else
    throw std::string ("ERROR: Unrecognized argument '") + positional[0] + "'";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
// taskd suspend org   <org>
// taskd suspend group <org> <group>
// taskd suspend user  <org> <user>
int command_suspend (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  std::string root = "";

  std::vector <std::string> positional;
  bool verbose = true;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--data",   *i, 3))  root = *(++i);
    else if (closeEnough ("--quiet",  *i, 3))  verbose = false;
    else if (closeEnough ("--debug",  *i, 3))  ; // TODO Is this necessary?
    else if (taskd_applyOverride (config, *i)) ;
    else
      positional.push_back (*i);
  }

  if (root == "")
  {
    char* root_env = getenv ("TASKDDATA");
    if (root_env)
      root = root_env;
  }

  // Verify that root exists.
  if (root == "")
    throw std::string ("ERROR: The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  if (positional.size () < 1)
    throw std::string ("ERROR: Subcommand not specified - expected 'org', 'group' or 'user'.");

  // Suspend an organization.
  //   org <org>
  if (closeEnough ("org", positional[0], 3))
  {
    if (positional.size () < 2)
      throw std::string ("Usage: taskd suspend [options] org <org>");

    for (unsigned int i = 1; i < positional.size (); ++i)
    {
      if (! taskd_is_org (root_dir, positional[i]))
        throw std::string ("ERROR: Organization '") + positional[i] + "' does not exist.";

      if (suspend_node (root_dir._data + "/orgs/" + positional[i]))
      {
        if (verbose)
          std::cout << "Suspended organization '" << positional[i] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to suspend organization '") + positional[i] + "'.";
    }
  }

  // Suspend a group.
  //   group <org> <group>
  else if (closeEnough ("group", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd suspend [options] group <org> <group>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_group (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: Group '") + positional[i] + "' does not exist.";

      if (suspend_node (root_dir._data + "/orgs/" + positional[1] + "/groups/" + positional[i]))
      {
        if (verbose)
          std::cout << "Suspended group '" << positional[i] << "' in organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to suspend group '") + positional[i] + "'.";
    }
  }

  // Suspend a user.
  //   user <org> <user>
  else if (closeEnough ("user", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd suspend [options] user <org> <user>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_user (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: User '") + positional[i] + "' does not  exists.";

      if (suspend_node (root_dir._data + "/orgs/" + positional[1] + "/users/" + positional[i]))
      {
        if (verbose)
          std::cout << "Suspended user '" << positional[i] << "' in organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to suspend user '") + positional[i] + "'.";
    }
  }

  else
    throw std::string ("ERROR: Unrecognized argument '") + positional[0] + "'";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
// taskd resume org   <org>
// taskd resume group <org> <group>
// taskd resume user  <org> <user>
int command_resume (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  std::string root = "";

  std::vector <std::string> positional;
  bool verbose = true;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--data",   *i, 3))  root = *(++i);
    else if (closeEnough ("--quiet",  *i, 3))  verbose = false;
    else if (closeEnough ("--debug",  *i, 3))  ; // TODO Is this necessary?
    else if (taskd_applyOverride (config, *i)) ;
    else
      positional.push_back (*i);
  }

  if (root == "")
  {
    char* root_env = getenv ("TASKDDATA");
    if (root_env)
      root = root_env;
  }

  // Verify that root exists.
  if (root == "")
    throw std::string ("ERROR: The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  if (positional.size () < 1)
    throw std::string ("ERROR: Subcommand not specified - expected 'org', 'group' or 'user'.");

  // Resume an organization.
  //   org <org>
  if (closeEnough ("org", positional[0], 3))
  {
    if (positional.size () < 2)
      throw std::string ("Usage: taskd resume [options] org <org>");

    for (unsigned int i = 1; i < positional.size (); ++i)
    {
      if (! taskd_is_org (root_dir, positional[i]))
        throw std::string ("ERROR: Organization '") + positional[i] + "' does not exist.";

      if (resume_node (root_dir._data + "/orgs/" + positional[i]))
      {
        if (verbose)
          std::cout << "Resumed organization '" << positional[i] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to resume organization '") + positional[i] + "'.";
    }
  }

  // Resume a group.
  //   group <org> <group>
  else if (closeEnough ("group", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd resume [options] group <org> <group>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_group (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: Group '") + positional[i] + "' does not exist.";

      if (resume_node (root_dir._data + "/orgs/" + positional[1] + "/groups/" + positional[i]))
      {
        if (verbose)
          std::cout << "Resumed group '" << positional[i] << "' in organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to resume group '") + positional[i] + "'.";
    }
  }

  // Resume a user.
  //   user <org> <user>
  else if (closeEnough ("user", positional[0], 3))
  {
    if (positional.size () < 3)
      throw std::string ("Usage: taskd resume [options] user <org> <user>");

    if (! taskd_is_org (root_dir, positional[1]))
      throw std::string ("ERROR: Organization '") + positional[1] + "' does not exist.";

    for (unsigned int i = 2; i < positional.size (); ++i)
    {
      if (! taskd_is_user (root_dir, positional[1], positional[i]))
        throw std::string ("ERROR: User '") + positional[i] + "' does not  exists.";

      if (resume_node (root_dir._data + "/orgs/" + positional[1] + "/users/" + positional[i]))
      {
        if (verbose)
          std::cout << "Resumed user '" << positional[i] << "' in organization '" << positional[1] << "'\n";
      }
      else
        throw std::string ("ERROR: Failed to resume user '") + positional[i] + "'.";
    }
  }

  else
    throw std::string ("ERROR: Unrecognized argument '") + positional[0] + "'";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
