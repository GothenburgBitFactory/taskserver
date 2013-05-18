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
#include <Directory.h>
#include <taskd.h>
#include <text.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
// taskd config --data <root> [<name> [<value>]]
int command_config (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  std::string root;
  std::string name;
  std::string value;
  bool verbose = true;
  bool debug = false;
  bool nonNull = false;
  bool confirmation = true;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--data",  *i, 3))   root  = *(++i);
    else if (closeEnough ("--force", *i, 3))   confirmation = false;
    else if (taskd_applyOverride (config, *i)) ;
    else if (name == "")                       name = *i;
    else if (value == "")
    {
      nonNull = true;
      if (value != "")
        value += ' ';

      value += *i;
    }
  }

  if (root == "")
  {
    char* root_env = getenv ("TASKDDATA");
    if (root_env)
      root = root_env;
  }

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  // Load the config file.
  config.load (root_dir._data + "/config");
  config.set ("root", root_dir._data);

  if (name != "" && nonNull)
  {
    bool change = false;

    // Read .taskd/config
    std::vector <std::string> contents;
    File::read (config._original_file, contents);

    bool found = false;
    std::vector <std::string>::iterator line;
    for (line = contents.begin (); line != contents.end (); ++line)
    {
      // If there is a comment on the line, it must follow the pattern.
      std::string::size_type comment = line->find ("#");
      std::string::size_type pos     = line->find (name + "=");

      if (pos != std::string::npos &&
          (comment == std::string::npos ||
           comment > pos))
      {
        found = true;
        if (!confirmation ||
            confirm (format ("Are you sure you want to change the value of '{1}' from '{2}' to '{3}'?",
                             name, config.get (name), value)))
        {
          if (comment != std::string::npos)
            *line = name + "=" + value + " " + line->substr (comment);
          else
            *line = name + "=" + value;

          change = true;
        }
      }
    }

    // Not found, so append instead.
    if (!found &&
        (!confirmation ||
         confirm (format ("Are you sure you want to add '{1}' with a value of '{2}'?", name, value))))
    {
      contents.push_back (name + "=" + value);
      change = true;
    }

    // Write .taskd (or equivalent)
    if (change)
    {
      File::write (config._original_file, contents);
      std::cout << format ("Config file {1} modified.",
                           config._original_file._data)
                << "\n";
    }
    else
      std::cout << "No changes made.\n";
  }
  else if (name != "")
  {
    // Read .taskd/config
    std::vector <std::string> contents;
    File::read (config._original_file, contents);

    bool change = false;
    bool found = false;
    std::vector <std::string>::iterator line;
    for (line = contents.begin (); line != contents.end (); ++line)
    {
      // If there is a comment on the line, it must follow the pattern.
      std::string::size_type comment = line->find ("#");
      std::string::size_type pos     = line->find (name + "=");

      if (pos != std::string::npos &&
          (comment == std::string::npos ||
           comment > pos))
      {
        found = true;

        // Remove name
        if (!confirmation ||
            confirm (format ("Are you sure you want to remove '{1}'?", name)))
        {
          *line = "";
          change = true;
        }
      }
    }

    if (!found)
      throw format ("ERROR: No entry named '{1}' found.", name);

    // Write .taskd (or equivalent)
    if (change)
    {
      File::write (config._original_file, contents);
      std::cout << format ("Config file {1} modified.",
                           config._original_file._data)
                << "\n";
    }
    else
      std::cout << "No changes made.\n";
  }

  else
  {
    std::cout << "\nConfiguration read from " << config._original_file._data << "\n\n";

    std::vector <std::string> names;
    config.all (names);
    std::map <std::string, std::string> data;

    std::vector <std::string>::iterator i;
    for (i = names.begin (); i != names.end (); ++i)
      data[*i] = config.get (*i);

    taskd_renderMap (data, "Variable", "Value");
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
