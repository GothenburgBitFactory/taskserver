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

//#include <sstream>
//#include <stdlib.h>
//#include <math.h>
//#include <algorithm>
//#include <Nibbler.h>
//#include <Date.h>
#include <Duration.h>
#include <Task.h>
#include <JSON.h>
//#include <RX.h>
#include <text.h>
#include <util.h>
//#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
Task::Task ()
: id (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const Task& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
Task& Task::operator= (const Task& other)
{
  if (this != &other)
  {
    std::map <std::string, std::string>::operator= (other);
    id = other.id;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// The uuid and id attributes must be exempt from comparison.
bool Task::operator== (const Task& other)
{
  if (size () != other.size ())
    return false;

  Task::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    if (i->first != "uuid" &&
        i->second != other.get (i->first))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Task::Task (const std::string& input)
{
  id = 0;
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Task::~Task ()
{
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::textToStatus (const std::string& input)
{
       if (input[0] == 'p') return Task::pending;
  else if (input[0] == 'c') return Task::completed;
  else if (input[0] == 'd') return Task::deleted;
  else if (input[0] == 'r') return Task::recurring;
  else if (input[0] == 'w') return Task::waiting;

  return Task::pending;
}

////////////////////////////////////////////////////////////////////////////////
std::string Task::statusToText (Task::status s)
{
       if (s == Task::pending)   return "pending";
  else if (s == Task::completed) return "completed";
  else if (s == Task::deleted)   return "deleted";
  else if (s == Task::recurring) return "recurring";
  else if (s == Task::waiting)   return "waiting";

  return "pending";
}

////////////////////////////////////////////////////////////////////////////////
void Task::setModified ()                                                       
{                                                                               
  char now[16];                                                                 
  sprintf (now, "%u", (unsigned int) time (NULL));                              
  set ("modified", now);                                                        
}                                                                               

////////////////////////////////////////////////////////////////////////////////
bool Task::has (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Task::all ()
{
  std::vector <std::string> all;
  Task::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    all.push_back (i->first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Task::get (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
time_t Task::get_date (const std::string& name) const
{
  Task::const_iterator i = this->find (name);
  if (i != this->end ())
    return (time_t) strtoul (i->second.c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, const std::string& value)
{
  (*this)[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
void Task::set (const std::string& name, int value)                             
{                                                                               
  (*this)[name] = format (value);                                               
}                                                                               

////////////////////////////////////////////////////////////////////////////////
void Task::remove (const std::string& name)
{
  Task::iterator it;
  if ((it = this->find (name)) != this->end ())
    this->erase (it);
}

////////////////////////////////////////////////////////////////////////////////
Task::status Task::getStatus () const
{
  return textToStatus (get ("status"));
}

////////////////////////////////////////////////////////////////////////////////
void Task::setStatus (Task::status status)
{
  set ("status", statusToText (status));
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Task::parse, and in the event of an error
// try a legacy parse (F3, FF2).  Note that FF1 is no longer supported.
//
// start --> [ --> Att --> ] --> end
//              ^       |
//              +-------+
//
void Task::parse (const std::string& input)
{
  std::string copy;
  if (input[input.length () - 1] == '\n')
    copy = input.substr (0, input.length () - 1);
  else
    copy = input;

  clear ();

  Nibbler n (copy);
  std::string line;
  if (n.skip     ('[')       &&
      n.getUntil (']', line) &&
      n.skip     (']')       &&
      n.depleted ())
  {
    if (line.length () == 0)
      throw std::string ("Empty record in input.");

    Nibbler nl (line);
    std::string name;
    std::string value;
    while (!nl.depleted ())
    {
      if (nl.getUntil (':', name) &&
          nl.skip (':')           &&
          nl.getQuoted ('"', value))
      {
        // Experimental legacy value translation of 'recur:m' --> 'recur:mo'.
        if (name == "recur" &&
            digitsOnly (value.substr (0, value.length () - 1)) &&
            value[value.length () - 1] == 'm')
          value += 'o';

        (*this)[name] = decode (json::decode (value));
      }

      nl.skip (' ');
    }

    std::string remainder;
    nl.getUntilEOS (remainder);
    if (remainder.length ())
      throw std::string ("Unrecognized characters at end of line.");
  }
  else
    throw std::string ("Record not recognized as format 4.");
}

////////////////////////////////////////////////////////////////////////////////
// The format is:
//
//   [ <name>:<value> ... ] \n
//
std::string Task::composeF4 () const
{
  std::string ff4 = "[";

  bool first = true;
  Task::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
  {
    if (it->second != "")
    {
      ff4 += (first ? "" : " ")
           + it->first
           + ":\"" + encode (json::encode (it->second)) + "\"";
      first = false;
    }
  }

  ff4 += "]\n";
  return ff4;
}

////////////////////////////////////////////////////////////////////////////////
// The purpose of Task::validate is three-fold:
//   1) To provide missing attributes where possible
//   2) To provide suitable warnings about odd states
//   3) To generate errors when the inconsistencies are not fixable
//
void Task::validate ()
{
  Task::status status = getStatus ();

  // 1) Provide missing attributes where possible
  // Provide a UUID if necessary.
  if (! has ("uuid"))
    set ("uuid", uuid ());

  // Recurring tasks get a special status.
  if (status == Task::pending &&
      has ("due")             &&
      has ("recur")           &&
      ! has ("parent"))
    status = Task::recurring;

  // Tasks with a wait: date get a special status.
  else if (status == Task::pending &&
           has ("wait"))
    status = Task::waiting;

  // By default, tasks are pending.
  else if (! has ("status"))
    status = Task::pending;

  // Store the derived status.
  setStatus (status);

/*
  // Provide an entry date unless user already specified one.
  if (!has ("entry"))
    setEntry ();
*/

/*
  // Completed tasks need an end date, so inherit the entry date.
  if (! has ("end") &&
      (getStatus () == Task::completed ||
       getStatus () == Task::deleted))
    setEnd ();
*/

  // 2) To provide suitable warnings about odd states

  // 3) To generate errors when the inconsistencies are not fixable

  // There is no fixing a missing description.
  if (!has ("description"))
    throw std::string ("A task must have a description.");
  else if (get ("description") == "")
    throw std::string ("Cannot add a task that is blank.");

  // Cannot have a recur frequency with no due date - when would it recur?
  if (! has ("due") && has ("recur"))
    throw std::string ("A recurring task must also have a 'due' date.");

  // Recur durations must be valid.
  if (has ("recur"))
  {
    Duration d;
    if (! d.valid (get ("recur")))
      throw std::string (format ("The recurrence value '{1}' is not valid.", get ("recur")));
  }

  // Priorities must be valid.
  if (has ("priority"))
  {
    std::string priority = get ("priority");
    if (priority != "H" &&
        priority != "M" &&
        priority != "L")
      throw format ("Priority values may be 'H', 'M' or 'L', not '{1}'.", priority);
  }
}

////////////////////////////////////////////////////////////////////////////////
