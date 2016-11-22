////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2016, Göteborg Bit Factory.
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
#include <FS.h>
#include <Color.h>
#include <Task.h>
#include <RX.h>
#include <format.h>
#include <util.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
std::map <int, std::string> errors {
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

////////////////////////////////////////////////////////////////////////////////
bool taskd_applyOverride (Config& config, const std::string& arg)
{
  // If the arg looks like '--NAME=VALUE' or '--NAME:VALUE', apply it to config.
  if (arg.substr (0, 2) == "--")
  {
    auto equal = arg.find ('=', 2);
    if (equal == std::string::npos)
      equal = arg.find (':', 2);

    if (equal != std::string::npos &&
        equal > 2)
    {
      auto name  = arg.substr (2, equal - 2);
      auto value = arg.substr (equal + 1);

      config.set (name, value);
      return true;
    }
  }

  return false;
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
bool taskd_at_least (const std::string&, const std::string&)
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
                << '\n';

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_sendMessage (
  Config& config,
  const std::string& to,
  const Msg& out,
  Msg& in)
{
  auto destination = config.get (to);
  auto colon = destination.rfind (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + destination + "'";

  auto server      = destination.substr (0, colon);
  auto port        = destination.substr (colon + 1);

  auto ca          = config.get ("ca.cert");
  auto certificate = config.get ("api.cert");
  auto key         = config.get ("api.key");
  auto ciphers     = config.get ("ciphers");

  try
  {
    TLSClient client;
    client.debug (config.getInteger ("debug.tls"));

    auto trust_level = config.get ("trust");
    client.trust (trust_level == "allow all"       ? TLSClient::allow_all       :
                  trust_level == "ignore hostname" ? TLSClient::ignore_hostname :
                                                     TLSClient::strict);
    client.ciphers (ciphers);
    client.init (ca, certificate, key);
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
    auto max1 = title1.length ();
    auto max2 = title2.length ();

    for (auto& i : data)
    {
      if (i.first.length ()  > max1) max1 = i.first.length ();
      if (i.second.length () > max2) max2 = i.second.length ();
    }

    std::cout << std::left
              << std::setfill (' ')
              << std::setw (max1) << title1
              << "  "
              << std::setw (max2) << title2
              << '\n'
              << std::setfill ('-')
              << std::setw (max1) << ""
              << "  "
              << std::setw (max2) << ""
              << '\n';

    for (auto& i : data)
      std::cout << std::left
                << std::setfill (' ')
                << std::setw (max1) << i.first
                << "  "
                << std::setw (max2) << i.second
                << '\n';

    std::cout << '\n';
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
bool taskd_is_user (
  const Directory& root,
  const std::string& org,
  const std::string& user)
{
  Directory d (root);
  d += "orgs";
  d += org;
  d += "users";

  for (auto& u : d.list ())
  {
    Path cfg (u);
    cfg += "config";

    Config conf (cfg._data);
    if (conf.get ("user") == user)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool taskd_is_user_key (
  const Directory& root,
  const std::string& org,
  const std::string& key)
{
  Directory d (root);
  d += "orgs";
  d += org;
  d += "users";
  d += key;
  return d.exists ();
}

////////////////////////////////////////////////////////////////////////////////
std::string taskd_error (const int code)
{
  auto e = errors.find (code);
  if (e != errors.end ())
    return e->second;

  return "[Missing error code]";
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
