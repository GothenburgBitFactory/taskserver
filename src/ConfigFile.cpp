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
#include <stdlib.h>
#include <inttypes.h>
#include <Date.h>
#include <ConfigFile.h>
#include <text.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
// In all real use cases, Config::load is called.
Config::Config ()
: _original_file ()
, _dirty (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
  load (file);
}

////////////////////////////////////////////////////////////////////////////////
Config::Config (const Config& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
Config& Config::operator= (const Config& other)
{
  if (this != &other)
  {
    std::map<std::string, std::string>::operator= (other);

    _original_file = other._original_file;
    _dirty         = other._dirty;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Read the Configuration file and populate the *this map.  The file format is
// simply lines with name=value pairs.  Whitespace between name, = and value is
// not tolerated, but blank lines and comments starting with # are allowed.
//
// Nested files are now supported, with the following construct:
//   include /absolute/path/to/file
//
void Config::load (const std::string& file, int nest /* = 1 */)
{
  if (nest > 10)
    throw std::string ("ERROR: Configuration file nested to more than 10 levels deep - this has to be a mistake.");

  // First time in, load the default values.
  if (nest == 1)
    _original_file = File (file);

  // Read the file, then parse the contents.
  std::string contents;
  if (File::read (file, contents) && contents.length ())
    parse (contents, nest);
}

////////////////////////////////////////////////////////////////////////////////
// Write the Configuration file.
void Config::save ()
{
  std::string contents;
  std::map <std::string, std::string>::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    contents += i->first + "=" + i->second + "\n";

  File::write (_original_file, contents);
  _dirty = false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::parse (const std::string& input, int nest /* = 1 */)
{
  // Shortcut case for default constructor.
  if (input.length () == 0)
    return;

  // Split the input into lines.
  std::vector <std::string> lines;
  split (lines, input, "\n");

  // Parse each line.
  std::vector <std::string>::iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    std::string line = *it;

    // Remove comments.
    std::string::size_type pound = line.find ("#"); // no i18n
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    line = trim (line, " \t"); // no i18n

    // Skip empty lines.
    if (line.length () > 0)
    {
      std::string::size_type equal = line.find ("="); // no i18n
      if (equal != std::string::npos)
      {
        std::string key   = trim (line.substr (0, equal), " \t"); // no i18n
        std::string value = trim (line.substr (equal+1, line.length () - equal), " \t"); // no i18n

        (*this)[key] = value;
      }
      else
      {
        std::string::size_type include = line.find ("include"); // no i18n.
        if (include != std::string::npos)
        {
          Path included (trim (line.substr (include + 7), " \t"));
          if (included.is_absolute ())
          {
            if (included.readable ())
              this->load (included, nest + 1);
            else
              throw format ("ERROR: Could not read include file '{1}'.", included._data);
          }
          else
            throw format ("ERROR: Can only include files with absolute paths, not '{1}'", included._data);
        }
        else
          throw format ("ERROR: Malformed entry '{1}' in config file.", line);
      }
    }
  }

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string Config::get (const std::string& key)
{
  return (*this)[key];
}

////////////////////////////////////////////////////////////////////////////////
const int Config::getInteger (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
    return strtoimax ((*this)[key].c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
const double Config::getReal (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
    return strtod ((*this)[key].c_str (), NULL);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
const bool Config::getBoolean (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
  {
    std::string value = lowerCase ((*this)[key]);
    if (value == "true"   ||
        value == "1"      ||
        value == "y"      ||
        value == "yes"    ||
        value == "on")
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const int value)
{
  (*this)[key] = format (value);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const double value)
{
  (*this)[key] = format (value, 1, 8);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const std::string& value)
{
  (*this)[key] = value;
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Autovivification is ok here.
void Config::setIfBlank (const std::string& key, const std::string& value)
{
  if ((*this)[key] == "") 
  {
    (*this)[key] = value;
    _dirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Provide a vector of all configuration keys.
void Config::all (std::vector<std::string>& items) const
{
  std::map <std::string, std::string>::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
    items.push_back (it->first);
}

////////////////////////////////////////////////////////////////////////////////
bool Config::dirty ()
{
  return _dirty;
}

////////////////////////////////////////////////////////////////////////////////
