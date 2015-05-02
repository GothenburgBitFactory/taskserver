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
#include <stdlib.h>
#include <ConfigFile.h>
#include <Directory.h>
#include <taskd.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
// taskd config --data <root> [<name> [<value>]]
void command_config (Database& db, const std::vector <std::string>& args)
{
  bool verbose      = db._config->getBoolean ("verbose");
  bool confirmation = db._config->getBoolean ("confirmation");

  std::string name;
  std::string value;
  bool nonNull = false;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
    if (name == "")
    {
      name = *i;
    }
    else if (value == "")
    {
      nonNull = true;
      if (value != "")
        value += ' ';

      value += *i;
    }
  }

  std::string root = db._config->get ("root");
  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string (STRING_CONFIG_NO_PATH);

  // Load the config file.
  Config config;
  config.load (root_dir._data + "/config");
  config.set ("root", root_dir._data);

  // taskd config <name> <value>
  // - set <name> to <value>
  if (name != "" && nonNull)
  {
    if (! config._original_file.writable ())
      throw std::string (STRING_CONFIG_READ_ONLY);

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
           comment > pos) &&
          trim (*line, " \t").find (name + "=") == 0)
      {
        found = true;
        if (!confirmation ||
            confirm (format (STRING_CONFIG_OVERWRITE, name, config.get (name), value)))
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
         confirm (format (STRING_CONFIG_ADD, name, value))))
    {
      contents.push_back (name + "=" + value);
      change = true;
    }

    // Write .taskd (or equivalent)
    if (change)
    {
      File::write (config._original_file, contents);
      if (verbose)
        std::cout << format (STRING_CONFIG_MODIFIED, config._original_file._data)
                  << "\n";
    }
    else
    {
      if (verbose)
        std::cout << STRING_CONFIG_NO_CHANGE << "\n";
    }
  }

  // taskd config <name>
  // - remove <name>
  else if (name != "")
  {
    if (! config._original_file.writable ())
      throw std::string (STRING_CONFIG_READ_ONLY);

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
            confirm (format (STRING_CONFIG_REMOVE, name)))
        {
          *line = "";
          change = true;
        }
      }
    }

    if (!found)
      throw format (STRING_CONFIG_NOT_FOUND, name);

    // Write .taskd (or equivalent)
    if (change)
    {
      if (! config._original_file.exists ())
        config._original_file.create (0600);

      File::write (config._original_file, contents);
      if (verbose)
        std::cout << format (STRING_CONFIG_MODIFIED,
                             config._original_file._data)
                  << "\n";
    }
    else
    {
      if (verbose)
        std::cout << STRING_CONFIG_NO_CHANGE
                  << "\n";
    }
  }

  // taskd config
  // - list all settings.
  else
  {
    if (verbose)
      std::cout << "\n"
                << format (STRING_CONFIG_SOURCE, config._original_file._data)
                << "\n\n";
    taskd_renderMap (config, "Variable", "Value");
  }
}

////////////////////////////////////////////////////////////////////////////////
