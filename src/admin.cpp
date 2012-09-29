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

#include <taskd.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
static bool is_org (
  const Directory& root,
  const std::string& org)
{
  Directory d (root);
  d += "orgs";
  d += org;
  return d.exists ();
}

////////////////////////////////////////////////////////////////////////////////
static bool is_group (
  const Directory& root,
  const std::string& org,
  const std::string& group)
{
  Directory d (root);
  d += "orgs";
  d += org;
  d += "groups";
  d += group;
  return d.exists ();
}

////////////////////////////////////////////////////////////////////////////////
static bool is_user (
  const Directory& root,
  const std::string& org,
  const std::string& user)
{
  Directory d (root);
  d += "orgs";
  d += org;
  d += "users";
  d += user;
  return d.exists ();
}

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

  return new_org.create () &&
         groups.create () &&
         users.create ();
}

////////////////////////////////////////////////////////////////////////////////
static bool add_group (
  const Directory& root,
  const std::string& org,
  const std::string& group)
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool add_user (
  const Directory& root,
  const std::string& org,
  const std::string& user)
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// taskd add {org,group,user} name
int command_add (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  std::string root = "";

  std::vector <std::string> positional;

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (taskd_applyOverride (config, *i))   ;
    else
      positional.push_back (*i);
  }

  // Verify that root exists.
  if (root == "")
    throw std::string ("The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("The '--data' path does not exist.");

  if (positional.size () < 1)
    throw std::string ("ERROR: Subcommand not specified - expected 'org', 'group' or 'user'.");

  if (closeEnough ("org", positional[0], 3))
  {
    if (positional.size () < 2)
      throw std::string ("ERROR: Organization name(s) not specified.");

    for (int i = 1; i < positional.size (); ++i)
    {
      if (is_org (root_dir, positional[i]))
        throw std::string ("ERROR: Organization '") + positional[1] + "' already exists.";

      if (!add_org (root_dir, positional[i]))
        throw std::string ("ERROR: Failed to create organization '") + positional[1] + "'.";
    }
  }

/*
  else if (closeEnough ("group", positional[0], 3))
  {
  }

  else if (closeEnough ("user", positional[0], 3))
  {
  }
*/

  else
    throw std::string ("ERROR: Unrecognized argument '") + positional[0] + "'";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int command_remove (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  std::string root = "";

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (taskd_applyOverride (config, *i))   ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";


  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int command_suspend (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  std::string root = "";

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (taskd_applyOverride (config, *i))   ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";


  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int command_resume (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  std::string root = "";

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (taskd_applyOverride (config, *i))   ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";


  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
