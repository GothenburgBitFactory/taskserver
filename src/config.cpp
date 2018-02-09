////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2018, Göteborg Bit Factory.
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
#include <FS.h>
#include <format.h>
#include <taskd.h>
#include <shared.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
// taskd config --data <root> [<name> [<value>]]
void command_config (Database& db, const std::vector <std::string>& args)
{
  auto verbose      = db._config->getBoolean ("verbose");
  auto confirmation = db._config->getBoolean ("confirmation");

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

  auto root = db._config->get ("root");
  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  // Load the config file.
  Config config;
  config.load (root_dir._data + "/config");
  config.set ("root", root_dir._data);

  // taskd config <name> <value>
  // - set <name> to <value>
  if (name != "" && nonNull)
  {
    if (! config._original_file.writable ())
      throw std::string ("Configuration file is read-only, no changes possible.");

    auto change = false;

    // Read .taskd/config
    std::vector <std::string> contents;
    File::read (config._original_file, contents);

    auto found = false;
    for (auto& line : contents)
    {
      // If there is a comment on the line, it must follow the pattern.
      auto comment = line.find ("#");
      auto pos     = line.find (name + "=");

      if (pos != std::string::npos &&
          (comment == std::string::npos ||
           comment > pos) &&
          trim (line, " \t").find (name + "=") == 0)
      {
        found = true;
        if (!confirmation ||
            confirm (format ("Are you sure you want to change the value of '{1}' from '{2}' to '{3}'?", name, config.get (name), value)))
        {
          if (comment != std::string::npos)
            line = name + "=" + value + " " + line.substr (comment);
          else
            line = name + "=" + value;

          change = true;
        }
      }
    }

    // Not found, so append instead.
    if (! found &&
        (! confirmation ||
         confirm (format ("Are you sure you want to add '{1}' with a value of '{2}'?", name, value))))
    {
      contents.push_back (name + "=" + value);
      change = true;
    }

    // Write .taskd (or equivalent)
    if (change)
    {
      File::write (config._original_file, contents);
      if (verbose)
        std::cout << format ("Config file {1} modified.\n", config._original_file._data);
    }
    else
    {
      if (verbose)
        std::cout << "No changes made.\n";
    }
  }

  // taskd config <name>
  // - remove <name>
  else if (name != "")
  {
    if (! config._original_file.writable ())
      throw std::string ("Configuration file is read-only, no changes possible.");

    // Read .taskd/config
    std::vector <std::string> contents;
    File::read (config._original_file, contents);

    bool change = false;
    bool found = false;
    for (auto& line : contents)
    {
      // If there is a comment on the line, it must follow the pattern.
      auto comment = line.find ("#");
      auto pos     = line.find (name + "=");

      if (pos != std::string::npos &&
          (comment == std::string::npos ||
           comment > pos))
      {
        found = true;

        // Remove name
        if (!confirmation ||
            confirm (format ("Are you sure you want to remove '{1}'?", name)))
        {
          line = "";
          change = true;
        }
      }
    }

    if (!found)
      throw format ("ERROR: No entry named '{1}' found.", name);

    // Write .taskd (or equivalent)
    if (change)
    {
      if (! config._original_file.exists ())
        config._original_file.create (0600);

      File::write (config._original_file, contents);
      if (verbose)
        std::cout << format ("Config file {1} modified.", config._original_file._data)
                  << '\n';
    }
    else
    {
      if (verbose)
        std::cout << "No changes made.\n";
    }
  }

  // taskd config
  // - list all settings.
  else
  {
    if (verbose)
      std::cout << format ("\nConfiguration read from {1}\n\n", config._original_file._data);
    taskd_renderMap (config, "Variable", "Value");
  }
}

////////////////////////////////////////////////////////////////////////////////
