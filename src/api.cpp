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
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <stdio.h>
#include <inttypes.h>
#include <Socket.h>
#include <Directory.h>
#include <Date.h>
#include <Color.h>
#include <Log.h>
#include <text.h>
#include <taskd.h>
#include <cmake.h>

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
  if (d.create ())
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
  const Msg& out,
  bool spool /* = true */)
{
  std::string destination = config.get (to);
  std::string::size_type colon = destination.find (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  std::string server = destination.substr (0, colon);
  int port = strtoimax (destination.substr (colon + 1).c_str (), NULL, 10);

  try
  {
    Socket s (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s.connect (server, port);
    s.write (out.serialize () + "\r\n");

    std::string response;
    s.read (response);
    s.close ();

    Msg in;
    in.parse (response);

    // Indicate message sent.
    return true;
  }

  catch (std::string& error)
  {
    if (spool)
      taskd_spoolMessage (config, to, out);
  }

  // Indicate message spooled.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_sendMessage (
  Config& config,
  const std::string& to,
  const Msg& out,
  Msg& in,
  bool spool /* = true */)
{
  std::string destination = config.get (to);
  std::string::size_type colon = destination.find (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  std::string server = destination.substr (0, colon);
  int port = strtoimax (destination.substr (colon + 1).c_str (), NULL, 10);

  try
  {
    Socket s (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s.connect (server, port);
    s.write (out.serialize () + "\r\n");

    std::string response;
    s.read (response);
    s.close ();

    in.parse (response);

    // Indicate message sent.
    return true;
  }

  catch (std::string& error)
  {
    if (spool)
      taskd_spoolMessage (config, to, out);
  }

  // Indicate message spooled.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
void taskd_spoolMessage (Config& config, const std::string& to, const Msg& out)
{
  Directory outbox (
    Directory (config.get ("scm.config.dir")).parent () + "/.taskd/outbox");
  File spool (outbox._data + "/" + to + "." + Date ().toISO () + ".msg");
  spool.write (out.serialize ());
}
////////////////////////////////////////////////////////////////////////////////
bool taskd_resendMessage (Config& config, const std::string& msgFile)
{
  std::string to = "";
  std::string::size_type last_slash = msgFile.rfind ('/');
  if (last_slash != std::string::npos)
  {
    std::string::size_type second_last_dot = msgFile.rfind ('.');
    if (second_last_dot != std::string::npos)
    {
      second_last_dot = msgFile.rfind ('.', second_last_dot - 1);
      if (second_last_dot != std::string::npos)
        to = msgFile.substr (last_slash + 1, second_last_dot - last_slash - 1);
    }
  }

  if (to == "")
    return false;  // TODO Error?

  // Read the message from the file.
  std::string message;
  File::read (msgFile, message);

  std::string destination = config.get (to);
  std::string::size_type colon = destination.find (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  std::string server = destination.substr (0, colon);
  int port = strtoimax (destination.substr (colon + 1).c_str (), NULL, 10);

  try
  {
    Socket s (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s.connect (server, port);
    s.write (message + "\r\n");

    std::string response;
    s.read (response);
    s.close ();

    Msg in;
    in.parse (response);

    File::remove (msgFile);

    // Indicate message sent.
    return true;
  }

  catch (std::string& error)
  {
    // No need to spool, no need to delete.
  }

  // Indicate message not resent.
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
// In order to match, the two messages must not differ in the intersection.
bool taskd_match (
  const Msg& project,
  const Msg& satellite,
  Log& log,
  Config& config)
{
  // Get the contents of the project data.
  std::vector <std::string> names;
  project.all (names);

  // Look for corresponding satellite values that differ as a disqualification.
  std::vector <std::string>::iterator i;
  for (i = names.begin (); i != names.end (); ++i)
  {
    // Ignore meta-names.
    if (*i != "time" &&
        *i != "type" &&
        *i != "version")
    {
      if (satellite.get (*i) != "" &&
          project.get (*i) != satellite.get (*i))
      {
        if (config.getBoolean ("debug"))
          log.format ("DEBUG work mismatch %s/%s/%s <> %s/%s/%s",
                      satellite.get ("satellite.label").c_str (),
                      i->c_str (),
                      satellite.get (*i).c_str (),
                      project.get ("project.name").c_str (),
                      i->c_str (),
                      project.get (*i).c_str ());
        return false;
      }
    }
  }

  // No disqualifications - therefore a match.
  return true;
}

////////////////////////////////////////////////////////////////////////////////
