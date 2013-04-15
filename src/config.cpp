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

/*
#include <iostream>
#include <cstring>
*/
#include <ConfigFile.h>
/*
#include <File.h>
#include <cmake.h>
*/
#include <taskd.h>
/*
#include <text.h>
#include <util.h>
*/

////////////////////////////////////////////////////////////////////////////////
int command_config (Config& config, const std::vector <std::string>& args)
{
  int status = 0;
/*
  // Check prerequisites.
  taskd_requireConfiguration (config);

  // Support:
  //   taskd config name value    # set name to value
  //   taskd config name ""       # set name to blank
  //   taskd config name          # remove name
  std::string name  = "";
  std::string value = "";
  bool nonNull = false;
  if (args.size () > 1)
  {
    name = args[1];

    if (args.size () > 2)
    {
      nonNull = true;
      for (unsigned int i = 2; i < args.size (); ++i)
      {
        if (i > 2)
          value += " ";

        value += args[i];
      }
    }
  }

  if (name != "" && nonNull)
  {
    std::cout << "# config <name> <value> - overwrite.\n";
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
        if (confirm (format ("Are you sure you want to change the value of '{1}' from '{2}' to '{3}'?",
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
        confirm (format ("Are you sure you want to add '{1}' with a value of '{2}'?", name, value)))
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
    std::cout << "# config <name> - remove name/value.\n";

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
        if (confirm (format ("Are you sure you want to remove '{1}'?", name)))
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
*/

  return status;
}

////////////////////////////////////////////////////////////////////////////////
