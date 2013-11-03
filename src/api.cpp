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
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <stdio.h>
#include <inttypes.h>
#include <TLSClient.h>
#include <Directory.h>
#include <Date.h>
#include <Color.h>
#include <Log.h>
#include <Task.h>
#include <RX.h>
#include <text.h>
#include <util.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
static struct
{
  int code;
  std::string error;
} errors[] =
{
  // 2xx Success.
  { 200, "Ok" },
  { 201, "No change"},
  { 202, "Decline"},

  // 3xx Partial success.
  { 300, "Deprecated request type"},
  { 301, "Redirect"},
  { 302, "Retry"},

  // 4xx Client error.
  { 401, "Failure"},
  { 400, "Malformed data"},
  { 401, "Unsupported encoding"},
  { 420, "Server temporarily unavailable"},
  { 430, "Access denied"},
  { 431, "Account suspended"},
  { 432, "Account terminated"},

  // 5xx Server error.
  { 500, "Syntax error in request"},
  { 501, "Syntax error, illegal parameters"},
  { 502, "Not implemented"},
  { 503, "Command parameter not implemented"},
  { 504, "Request too big"},
};

#define NUM_ERRORS (sizeof (errors) / sizeof (errors[0]))

////////////////////////////////////////////////////////////////////////////////
bool taskd_applyOverride (Config& config, const std::string& arg)
{
  // If the arg looks like '--NAME=VALUE' or '--NAME:VALUE', apply it to config.
  if (arg.substr (0, 2) == "--")
  {
    std::string::size_type equal = arg.find ('=', 2);
    if (equal == std::string::npos)
      equal = arg.find (':', 2);

    if (equal != std::string::npos &&
        equal > 2)
    {
      std::string name  = arg.substr (2, equal - 2);
      std::string value = arg.substr (equal + 1);

      config.set (name, value);
      std::cout << "- Override " << name << '=' << value << "\n";
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int taskd_execute (const std::string& command, std::string& output)
{
  int status = 0;

  FILE* pipe = popen (command.c_str (), "r");
  if (pipe)
  {
    output = "";
    char line [1024] = {0};
    while (fgets (line, 1023, pipe))
      output += line;

    status = pclose (pipe);
  }
  else
    throw std::string ("ERROR: Could not execute '") + command + "'";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int taskd_runExtension (
  const std::string& pattern,
  const std::string& arguments,
  Config& config,
  bool verbose)
{
  int status = 0;

  std::string fqPattern = config.get ("extensions") + "/" + pattern;
  std::vector <std::string> paths = Path::glob (fqPattern);
  std::vector <std::string>::iterator plugin;
  for (plugin = paths.begin (); plugin != paths.end (); ++plugin)
  {
    if (Path (*plugin).executable ())
    {
      std::string command = *plugin + " " + arguments;
      if (verbose)
        std::cout << "- taskd_runExtension " << command << "\n";

      std::string out;
      status = taskd_execute (command, out);
      if (status == 0)
      {
        std::vector <std::string> lines;
        split (lines, out, '\n');
        std::vector <std::string>::iterator i;
        for (i = lines.begin (); i != lines.end (); ++i)
        {
          std::string::size_type colon = i->find (':');
          if (colon != std::string::npos)
          {
            std::string name = trim (i->substr (0, colon));
            std::string value = trim (i->substr (colon + 1));

            if (name == "WARNING")
              std::cout << Color ("yellow").colorize ("- " + name + ": " + value)
                        << "\n";
            else
            {
              config.set (name, value);
              if (verbose)
                std::cout << "    " << name << "=" << value << "\n";
            }
          }
        }

        break;
      }
    }
  }

  if (paths.size () == 0 && verbose)
    std::cout << "- taskd_runExtension '" << fqPattern << "' not found - continuing.\n";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int taskd_runExtension (
  const std::string& pattern,
  const std::string& arguments,
  std::string& output,
  Config& config)
{
  int status = 0;

  std::string fqPattern = config.get ("extensions") + "/" + pattern;
  std::vector <std::string> paths = Path::glob (fqPattern);
  std::vector <std::string>::iterator plugin;
  for (plugin = paths.begin (); plugin != paths.end (); ++plugin)
  {
    if (Path (*plugin).executable ())
    {
      std::string command = *plugin + " " + arguments;
      if (config.getBoolean ("verbose"))
        output += "--- taskd_runExtension " + command + "\n";

      status = taskd_execute (command, output);
      if (status == 0)
        break;
    }
  }

  if (paths.size () == 0 &&
      config.getBoolean ("verbose"))
    output += "--- taskd_runExtension '" + fqPattern + "' not found.\n";

  return status;
}

////////////////////////////////////////////////////////////////////////////////
int taskd_runHook (
  const std::string& hook,
  const std::string& arguments,
  Log& log,
  Config& config)
{
  int status = 0;

  std::string scripts = config.get (hook);
  if (scripts != "")
  {
    std::vector <std::string> hooks;
    split (hooks, scripts, ',');
    std::vector <std::string>::iterator h;
    for (h = hooks.begin (); h != hooks.end (); ++h)
    {
      if (Path (*h).executable () ||
          h->find ("taskd ") != std::string::npos)
      {
        std::string command = *h + " " + arguments;
        std::string output;
        status = taskd_execute (command, output);

        log.format ("%s: %s", hook.c_str (), command.c_str ());
        log.write (output, true);

        if (status != 0)
          break;
      }
      else
        log.format ("ERROR: Hook '%s' script '%s' not executable.",
                    hook.c_str (),
                    h->c_str ());
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
void taskd_requireSetting (Config& config, const std::string& name)
{
  if (config.get (name) == "")
    throw std::string ("ERROR: Required configuration setting '") + name + "' not found.";
}

////////////////////////////////////////////////////////////////////////////////
// Assert: message.version >= version
void taskd_requireVersion (const Msg& message, const std::string& version)
{
  if (! taskd_at_least (message.get ("version"), version))
    throw std::string ("ERROR: Need at least version ") + version;
}

////////////////////////////////////////////////////////////////////////////////
// Assert: message.protocol >= version
void taskd_requireHeader (
  const Msg& message,
  const std::string& name,
  const std::string& value)
{
  if (message.get (name) != value)
    throw format ("ERROR: Message {1} should be '{2}'", name, value);
}

////////////////////////////////////////////////////////////////////////////////
// Tests left >= right, where left and right are version number strings.
// Assumes all versions are Major.Minor.Patch[other], such as '1.0.0' or
// '1.0.0beta1'
bool taskd_at_least (const std::string& left, const std::string& right)
{
  // TODO Implement this.

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_createDirectory (Directory& d, bool verbose)
{
  if (d.create (0700))
  {
    if (verbose)
      std::cout << Color ("green").colorize (
                     "- Created directory " + std::string (d))
                << "\n";

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_sendMessage (
  Config& config,
  const std::string& to,
  const Msg& out)
{
  std::string destination = config.get (to);
  std::string::size_type colon = destination.rfind (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  std::string server = destination.substr (0, colon);
  std::string port   = destination.substr (colon + 1);

  try
  {
    TLSClient client;
    client.debug (config.getInteger ("debug.tls"));
/*
    client.limit (1024 * 1024);
*/
    client.trust (config.getBoolean ("server.trust"));
    client.init (config.get ("ca.cert"), config.get ("client.cert"), config.get ("client.key"));
    client.connect (server, port);

    client.send (out.serialize () + "\n");

    std::string response;
    client.recv (response);  // Ignored.
    client.bye ();

    // Indicate message sent.
    return true;
  }

  catch (std::string& error)
  {
  }

  // Indicate message spooled.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_sendMessage (
  Config& config,
  const std::string& to,
  const Msg& out,
  Msg& in)
{
  std::string destination = config.get (to);
  std::string::size_type colon = destination.rfind (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  std::string server = destination.substr (0, colon);
  std::string port   = destination.substr (colon + 1);

  try
  {
    TLSClient client;
    client.debug (config.getInteger ("debug.tls"));
/*
    client.limit (1024 * 1024);
*/
    client.trust (config.getBoolean ("server.trust"));
    client.init (config.get ("ca.cert"), config.get ("client.cert"), config.get ("client.key"));
    client.connect (server, port);

    client.send (out.serialize () + "\n");

    std::string response;
    client.recv (response);
    client.bye ();

    in.parse (response);

    // Indicate message sent.
    return true;
  }

  catch (std::string& error)
  {
  }

  // Indicate message spooled.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
void taskd_renderMap (
  const std::map <std::string, std::string>& data,
  const std::string& title1,
  const std::string& title2)
{
  if (data.size ())
  {
    unsigned int max1 = title1.length ();
    unsigned int max2 = title2.length ();

    std::map <std::string, std::string>::const_iterator i;
    for (i = data.begin (); i != data.end (); ++i)
    {
      if (i->first.length () > max1)  max1 = i->first.length ();
      if (i->second.length () > max2) max2 = i->second.length ();
    }

    std::cout << std::left 
              << std::setfill (' ')
              << std::setw (max1) << title1
              << "  "
              << std::setw (max2) << title2
              << "\n"
              << std::setfill ('-')
              << std::setw (max1) << ""
              << "  "
              << std::setw (max2) << ""
              << "\n";

    for (i = data.begin (); i != data.end (); ++i)
      std::cout << std::left
                << std::setfill (' ')
                << std::setw (max1) << i->first
                << "  "
                << std::setw (max2) << i->second
                << "\n";

    std::cout << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_is_org (
  const Directory& root,
  const std::string& org)
{
  Directory d (root);
  d += "orgs";
  d += org;
  return d.exists ();
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_is_group (
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
bool taskd_is_user (
  const Directory& root,
  const std::string& org,
  const std::string& password)
{
  Directory d (root);
  d += "orgs";
  d += org;
  d += "users";
  d += password;
  return d.exists ();
}

////////////////////////////////////////////////////////////////////////////////
std::string taskd_error (const int code)
{
  for (unsigned int i = 0; i < NUM_ERRORS; ++i)
    if (code == errors[i].code)
      return errors[i].error;

  return "[Missing error code]";
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_allow (
  const std::string& client,
  const std::vector <std::string>& allow,
  const std::vector <std::string>& deny,
  Log& log,
  bool debug /*= false*/)
{
  std::vector <std::string>::const_iterator i;
  for (i = deny.begin (); i != deny.end (); ++i)
  {
    // If deny contains 'none', pass.
    if (compare (*i, "none", false))
      break;

    // If deny contains 'all', fail and return.
    if (compare (*i, "all", false))
    {
      log.format ("INFO client '%s' denied by 'all'", client.c_str ());
      return false;
    }

    // If client is specifically denied.
    RX r (*i);
    if (r.match (client))
    {
      log.format ("INFO client '%s' denied", client.c_str ());
      return false;
    }
  }

  for (i = allow.begin (); i != allow.end (); ++i)
  {
    // If allow contains 'none', stop.
    if (compare (*i, "none", false))
    {
      log.format ("INFO client '%s' not allowed by 'none'", client.c_str ());
      return false;
    }

    // If allow contains 'all', or matches the client, pass.
    if (compare (*i, "all", false))
    {
      if (debug)
        log.format ("DEBUG client '%s' allowed by 'all'", client.c_str ());
      return true;
    }

    RX r (*i);
    if (r.match (client))
    {
      if (debug)
        log.format ("DEBUG client '%s' allowed", client.c_str ());
      return true;
    }
  }

  log.format ("WARNING client '%s' neither denied nor allowed.", client.c_str ());
  return false;
}

////////////////////////////////////////////////////////////////////////////////
void taskd_staticInitialize ()
{
  // List is corrected as of 2.3.0.
  // Note: annotation_* fields are missing, and are assumed to be 'string'
  // Note: UDA fields are missing, and are assumed to be 'string'

  Task::attributes["depends"]     = "string";
  Task::attributes["description"] = "string";
  Task::attributes["due"]         = "date";
  Task::attributes["end"]         = "date";
  Task::attributes["entry"]       = "date";
  Task::attributes["imask"]       = "numeric";
  Task::attributes["mask"]        = "string";
  Task::attributes["modified"]    = "date";
  Task::attributes["parent"]      = "string";
  Task::attributes["priority"]    = "string";
  Task::attributes["project"]     = "string";
  Task::attributes["recur"]       = "duration";
  Task::attributes["scheduled"]   = "date";
  Task::attributes["start"]       = "date";
  Task::attributes["status"]      = "string";
  Task::attributes["tags"]        = "string";
  Task::attributes["until"]       = "date";
  Task::attributes["uuid"]        = "string";
  Task::attributes["wait"]        = "date";
}

////////////////////////////////////////////////////////////////////////////////
