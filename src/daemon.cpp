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

#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <Server.h>
#include <Timer.h>
#include <Database.h>
#include <Log.h>
#include <Date.h>
#include <Duration.h>
#include <Color.h>
#include <Task.h>
#include <cmake.h>
#include <commit.h>
#include <text.h>
#include <util.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
class Daemon : public Server
{
public:
  Daemon (Config&);
  void handler (const std::string& input, std::string& output);

private:
  void handle_statistics (const Msg&, Msg&);
  void handle_sync       (const Msg&, Msg&);

private:
  void parse_payload (const std::string&, std::vector <std::string>&, std::string&) const;
  void load_server_data (const std::string&, const std::string&, std::vector <std::string>&) const;
  void append_server_data (const std::string&, const std::string&, const std::vector <std::string>&) const;
  unsigned int find_branch_point (const std::vector <std::string>&, const std::string&) const;
  void extract_subset (const std::vector <std::string>&, const unsigned int, std::vector <Task>&) const;
  bool contains (const std::vector <Task>&, const std::string&) const;
  std::string generate_payload (const std::vector <Task>&, const std::vector <std::string>&, const std::string&) const;
  unsigned int find_common_ancestor (const std::vector <std::string>&, unsigned int, const std::string&) const;
  void get_client_mods (std::vector <Task>&, const std::vector <std::string>&, const std::string&) const;
  void get_server_mods (std::vector <Task>&, const std::vector <std::string>&, const std::string&, unsigned int) const;
  void zipper_walk (const std::vector <Task>&, const std::vector <Task>&, Task&) const;
  time_t last_modification (const Task&) const;
  void patch (Task&, const Task&, const Task&) const;

public:
  Database _db;

private:
  Config& _config;
  Date _start;
  long _txn_count;
  long _error_count;
  double _busy;
  double _max_time;
  long _bytes_in;
  long _bytes_out;
};

////////////////////////////////////////////////////////////////////////////////
Daemon::Daemon (Config& settings)
: _db (&settings)
, _config (settings)
, _start (Date ())
, _txn_count (0)
, _error_count (0)
, _busy (0.0)
, _max_time (0.0)
, _bytes_in (0)
, _bytes_out (0)
{
}

////////////////////////////////////////////////////////////////////////////////
void Daemon::handler (const std::string& input, std::string& output)
{
  ++_txn_count;

  try
  {
    // Verify input is UTF8.  From RFC4627:
    //
    //   JSON text SHALL be encoded in Unicode.  The default encoding is
    //   UTF-8.
    //
    //   Since the first two characters of a JSON text will always be ASCII
    //   characters [RFC0020], it is possible to determine whether an octet
    //   stream is UTF-8, UTF-16 (BE or LE), or UTF-32 (BE or LE) by looking
    //   at the pattern of nulls in the first four octets.
    //
    //           00 00 00 xx  UTF-32BE
    //           00 xx 00 xx  UTF-16BE
    //           xx 00 00 00  UTF-32LE
    //           xx 00 xx 00  UTF-16LE
    //           xx xx xx xx  UTF-8
    if (input.length () >= 4 &&
        (!input[0]           ||
         !input[1]           ||
         !input[2]           ||
         !input[3]))
      throw 401;

    unsigned int request_limit = (unsigned) _config.getInteger ("request.limit");
    if (request_limit > 0 &&
        input.length () >= request_limit)
      throw 504;

    HighResTimer timer;
    timer.start ();

    // Request-specific processing here.
    Msg in;
    in.parse (input);
    Msg out;

    if (_config.getBoolean ("debug"))
    {
      // TODO dump incoming msg
    }

    // Handle or reject all message types.
    std::string type = in.get ("type");
         if (type == "statistics") handle_statistics (in, out);
    else if (type == "sync")       handle_sync       (in, out);
    else
      throw 500;

    output = out.serialize ();

    // Record response time.
    timer.stop ();
    double total = timer.total ();
    _busy += total;

    // Record high-water mark.
    if (total > _max_time)
      _max_time = total;
  }

  // Handlers can throw a status code, for a generic message.
  catch (int e)
  {
    ++_error_count;
    Msg err;
    err.set ("code", e);
    err.set ("status", taskd_error (e));
    output = err.serialize ();

    if (_log)
      _log->format ("[%d] ERROR %d %s", _txn_count, e, taskd_error (e).c_str ());
  }

  // Handlers can throw a string, for a 500 code with specific text.
  catch (std::string& e)
  {
    ++_error_count;
    Msg err;
    err.set ("code", 500);
    err.set ("status", e);
    output = err.serialize ();

    if (_log)
      _log->format ("[%d] %s", _txn_count, e.c_str ());
  }

  _bytes_in  += input.length ();
  _bytes_out += output.length ();
}

////////////////////////////////////////////////////////////////////////////////
// Statistics request from dev.
void Daemon::handle_statistics (const Msg& in, Msg& out)
{
  if (! _db.authenticate (in, out))
    return;

  // Support only task server protocol v1.
  taskd_requireHeader (in, "protocol", "v1");

  if (_log)
    _log->format ("[%d] 'statistics' from %s:%d",
                  _txn_count,
                  _client_address.c_str (),
                  _client_port);

  time_t uptime = Date () - _start;
  double idle = 0.0;
  if (uptime != 0)
    idle = 1.0 - (_busy / (double) uptime);

  int average_req          = 0;
  int average_resp         = 0;
  double average_resp_time = 0.0;
  double tps               = 0;
  if (_txn_count)
  {
    average_req       = _bytes_in  / _txn_count;
    average_resp      = _bytes_out / _txn_count;
    average_resp_time = _busy      / _txn_count;

    // Only calculate tps if average_resp_time is non-trivial.
    if (average_resp_time > 0.000001)
      tps = 1.0 / average_resp_time;
  }

  out.set ("uptime",                 (int) uptime);
  out.set ("transactions",           (int) _txn_count);
  out.set ("errors",                 (int) _error_count);
  out.set ("idle",                         idle);
  out.set ("total bytes in",         (int) _bytes_in);
  out.set ("total bytes out",        (int) _bytes_out);
  out.set ("average request bytes",  (int) average_req);
  out.set ("average response bytes", (int) average_resp);
  out.set ("average response time",        average_resp_time);
  out.set ("maximum response time",        _max_time);
  out.set ("tps",                          tps);

  out.set ("code",                         200);
  out.set ("status",                       taskd_error (200));
}

////////////////////////////////////////////////////////////////////////////////
// Sync request.
void Daemon::handle_sync (const Msg& in, Msg& out)
{
  if (! _db.authenticate (in, out))
    return;

  // Support only task server protocol v1.
  taskd_requireHeader (in, "protocol", "v1");

  // Note: org/user already validated during authentication.
  std::string org  = in.get ("org");
  std::string user = in.get ("user");

  if (_log)
    _log->format ("[%d] 'sync' from %s/%s at %s:%d",
                  _txn_count,
                  org.c_str (),
                  user.c_str (),
                  _client_address.c_str (),
                  _client_port);

  // Separate payload into client_data and client_key.
  std::vector <std::string> client_data;               // Incoming client data.
  std::string client_key;                              // Incoming client key.
  parse_payload (in.getPayload (), client_data, client_key);

  // Load all user data.
  std::vector <std::string> server_data;               // Data loaded on server.
  load_server_data (org, user, server_data);

  std::vector <std::string> new_server_data;           // New tasks for tx.data.
  std::vector <std::string> new_client_data;           // New tasks for client.

  // Find branch point and extract subset.
  unsigned int branch_point = find_branch_point (server_data, client_key);
  std::vector <Task> server_subset;
  extract_subset (server_data, branch_point, server_subset);

  // Maintain a list of already-merged task UUIDs.
  std::vector <std::string> already_seen;

  // For each incoming task...
  std::vector <std::string>::iterator client_task;
  for (client_task = client_data.begin ();
       client_task != client_data.end ();
       ++client_task)
  {
    // Validate task.
    Task task (*client_task);
    std::string uuid = task.get ("uuid");
    task.validate ();

    _log->format ("[%d] Validated: %s '%s'",
                  _txn_count,
                  uuid.c_str (),
                  task.get ("description").c_str ());

    // If task is in subset
    if (contains (server_subset, uuid))
    {
      // Merging a task causes a complete scan, and that picks up all mods to
      // that same task.  Therefore, there is no need to re-process a UUID.
      if (std::find (already_seen.begin (), already_seen.end (), uuid) != already_seen.end ())
        continue;

      already_seen.push_back (uuid);

      _log->format ("[%d] Merge needed", _txn_count);

      // Find common ancestor, prior to branch point
      unsigned int common_ancestor = find_common_ancestor (server_data,
                                                           branch_point,
                                                           uuid);
      _log->format ("[%d] Ancestor: %d %s", _txn_count, common_ancestor, server_data[common_ancestor].c_str ());

      // List the client-side modifications.
      std::vector <Task> client_mods;
      get_client_mods (client_mods, client_data, uuid);

      // List the server-side modifications.
      std::vector <Task> server_mods;
      get_server_mods (server_mods, server_data, uuid, common_ancestor);

      // Merge sort between client_mods and server_mods, patching ancestor.
      Task combined (server_data[common_ancestor]);
      zipper_walk (client_mods, server_mods, combined);
      std::string combined_F4 = trimRight (combined.composeF4 (), "\n");

      // Append combined task to client and server data, if not already there.
      new_server_data.push_back (combined_F4);
      new_client_data.push_back (combined_F4);
    }
    else
    {
      _log->format ("[%d] Store", _txn_count);

      // Task not in subset, therefore can be stored unmodified.  Does not get
      // returned to client.
      new_server_data.push_back (*client_task);
    }
  }

  // New server data means a new sync key must be generated.  No new server data
  // means the most recent sync key is reused.
  std::string new_client_key = "";
  if (new_server_data.size ())
  {
    new_client_key = uuid ();
    new_server_data.push_back (new_client_key);
    _log->format ("[%d] New sync key: %s", _txn_count, new_client_key.c_str ());

    // Append new_server_data to file.
    append_server_data (org, user, new_server_data);
  }
  else
  {
    std::vector <std::string>::reverse_iterator i;
    for (i = server_data.rbegin (); i != server_data.rend (); ++i)
      if ((*i)[0] != '[')
      {
        new_client_key = *i;
        break;
      }

    _log->format ("[%d] Using latest sync key: %s", _txn_count, new_client_key.c_str ());
  }

  // If there is outgoing data, generate payload + key.
  std::string payload = "";
  if (server_subset.size () ||
      new_client_data.size ())
  {
    payload = generate_payload (server_subset,
                                new_client_data,
                                new_client_key);
  }

  // No outgoing data, just sent the latest key.
  else
  {
    payload = new_client_key + "\n";
  }

  // Have payload and key, therefore success.
  if (payload        != "" &&
      new_client_key != "")
  {
    out.setPayload (payload);
    out.set ("code",   200);
    out.set ("status", taskd_error (200));
  }

  // Nothing changed --> 201 is a success code.
  else
  {
    _log->format ("[%d] No change", _txn_count);
    out.set ("code",   201);
    out.set ("status", taskd_error (201));
  }
}

////////////////////////////////////////////////////////////////////////////////
void Daemon::parse_payload (
  const std::string& payload,
  std::vector <std::string>& data,
  std::string& key) const
{
  // Break payload into lines.
  std::vector <std::string> lines;
  split (lines, payload, '\n');

  // Separate into data and key.
  // TODO Some syntax checking would be nice.
  std::vector <std::string>::iterator i;
  for (i = lines.begin (); i != lines.end (); ++i)
  {
    if (*i != "")
    {
      if ((*i)[0] == '[')
        data.push_back (*i);
      else
        key = *i;
    }
  }

  _log->format ("[%d] Client key: %s", _txn_count, key.c_str ());

  for (i = data.begin (); i != data.end (); ++i)
    _log->format ("[%d] Client data: %s", _txn_count, i->c_str ());
}

////////////////////////////////////////////////////////////////////////////////
void Daemon::load_server_data (
  const std::string& org,
  const std::string& user,
  std::vector <std::string>& data) const
{
  Directory user_dir (_config.get ("root"));
  user_dir += "orgs";
  user_dir += org;
  user_dir += "users";
  user_dir += user;
  File user_data (user_dir._data + "/tx.data");

  if (user_data.exists ())
    user_data.read (data);

  _log->format ("[%d] Read server data: %u line(s)", _txn_count, data.size ());
}

////////////////////////////////////////////////////////////////////////////////
void Daemon::append_server_data (
  const std::string& org,
  const std::string& user,
  const std::vector <std::string>& data) const
{
  Directory user_dir (_config.get ("root"));
  user_dir += "orgs";
  user_dir += org;
  user_dir += "users";
  user_dir += user;
  File user_data (user_dir._data + "/tx.data");

  user_data.append (data);

  _log->format ("[%d] Appended %u line(s) to server data", _txn_count, data.size ());
}

////////////////////////////////////////////////////////////////////////////////
// Note: A missing client_key implies first-time sync, which means the earliest
//       possible branch point is used.
unsigned int Daemon::find_branch_point (
  const std::vector <std::string>& data,
  const std::string& key) const
{
  unsigned int branch = 0;

  // A missing key is either a first-time sync, or a request to get all data.
  if (key == "")
    return branch;

  bool found = false;
  std::vector <std::string>::const_iterator i;
  for (i = data.begin (); i != data.end (); ++i)
  {
    if (*i == key)
    {
      found = true;
      break;
    }
    else
      ++branch;
  }

  if (!found)
    throw std::string ("Client sync key not found.");

  _log->format ("[%d] Branch point: %s --> %u", _txn_count, key.c_str (), branch);
  return branch;
}

////////////////////////////////////////////////////////////////////////////////
void Daemon::extract_subset (
  const std::vector <std::string>& data,
  const unsigned int branch_point,
  std::vector <Task>& subset) const
{
  if (branch_point < data.size ())
    for (unsigned int i = branch_point; i < data.size (); ++i)
      if (data[i][0] == '[')
        subset.push_back (Task (data[i]));

  _log->format ("[%d] Subset: %u line(s) after branch point", _txn_count, subset.size ());
}

////////////////////////////////////////////////////////////////////////////////
bool Daemon::contains (
  const std::vector <Task>& subset,
  const std::string& uuid) const
{
  std::vector <Task>::const_iterator i;
  for (i = subset.begin (); i != subset.end (); ++i)
    if (uuid == i->get ("uuid"))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Daemon::generate_payload (
  const std::vector <Task>& subset,
  const std::vector <std::string>& additions,
  const std::string& key) const
{
  std::string payload;

  std::vector <Task>::const_iterator t;
  for (t = subset.begin (); t != subset.end (); ++t)
    payload += t->composeF4 ();

  std::vector <std::string>::const_iterator s;
  for (s = additions.begin (); s != additions.end (); ++s)
    payload += *s + "\n";

  payload += key + "\n";

  return payload;
}

////////////////////////////////////////////////////////////////////////////////
// Starting at branch_point and working backwards, find the first instance of a
// task matching uuid.
unsigned int Daemon::find_common_ancestor (
  const std::vector <std::string>& data,
  unsigned int branch_point,
  const std::string& uuid) const
{
  for (int i = (int) branch_point; i >= 0; --i)
  {
    if (data[i][0] == '[')
    {
      Task t (data[i]);
      if (t.get ("uuid") == uuid)
        return (unsigned int) i;
    }
  }

  throw std::string ("ERROR: Could not find common ancestor for ") + uuid;
}

////////////////////////////////////////////////////////////////////////////////
// Extract tasks from the client list, with the given UUID, maintaining the
// sequence.
void Daemon::get_client_mods (
  std::vector <Task>& mods,
  const std::vector <std::string>& data,
  const std::string& uuid) const
{
  std::vector <std::string>::const_iterator line;
  for (line = data.begin (); line != data.end (); ++line)
  {
    if ((*line)[0] == '[')
    {
      Task t (*line);
      if (t.get ("uuid") == uuid)
        mods.push_back (t);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extract tasks from the server list, with the given UUID, maintaining the
// sequence.
void Daemon::get_server_mods (
  std::vector <Task>& mods,
  const std::vector <std::string>& data,
  const std::string& uuid,
  unsigned int ancestor) const
{
  for (unsigned int i = ancestor + 1; i < data.size (); ++i)
  {
    if (data[i][0] == '[')
    {
      Task t (data[i]);
      if (t.get ("uuid") == uuid)
        mods.push_back (t);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Simultaneously walks two lists, select either the left or the right depending
// on last modification time.
//
// What a horrible name.
void Daemon::zipper_walk (
  const std::vector <Task>& left,
  const std::vector <Task>& right,
  Task& combined) const
{
  std::vector <Task> dummy;
  dummy.push_back (combined);

  std::vector <Task>::const_iterator prev_l = dummy.begin ();
  std::vector <Task>::const_iterator iter_l = left.begin ();

  std::vector <Task>::const_iterator prev_r = dummy.begin ();
  std::vector <Task>::const_iterator iter_r = right.begin ();

  while (iter_l != left.end () &&
         iter_r != right.end ())
  {
    time_t mod_l = last_modification (*iter_l);
    time_t mod_r = last_modification (*iter_r);
    if (mod_l < mod_r)
    {
      _log->format ("[%d] applying left %d < %d", _txn_count, mod_l, mod_r);
      patch (combined, *prev_l, *iter_l);
      combined.set ("modified", (int) mod_l);
      prev_l = iter_l;
      ++iter_l;
    }
    else
    {
      _log->format ("[%d] applying right %d >= %d", _txn_count, mod_l, mod_r);
      patch (combined, *prev_r, *iter_r);
      combined.set ("modified", (int) mod_r);
      prev_r = iter_r;
      ++iter_r;
    }
  } 

  while (iter_l != left.end ())
  {
    patch (combined, *prev_l, *iter_l);
    combined.set ("modified", (int) last_modification (*iter_l));
    prev_l = iter_l;
    ++iter_l;
  }

  while (iter_r != right.end ())
  {
    patch (combined, *prev_r, *iter_r);
    combined.set ("modified", (int) last_modification (*iter_r));
    prev_r = iter_r;
    ++iter_r;
  }

  _log->format ("[%d] Zipper result %s", _txn_count, combined.composeF4 ().c_str ());
}

////////////////////////////////////////////////////////////////////////////////
// Get the last modication time for a task.  Ideally this is the attribute
// "modification".  If that is missing (pre taskwarrior 2.2.0), use the later of
// the "entry", "end", or"start" dates.
time_t Daemon::last_modification (const Task& task) const
{
  return task.has ("modified") ? task.get_date ("modified") :
         task.has ("end")      ? task.get_date ("end") :
         task.has ("start")    ? task.get_date ("start") :
                                 task.get_date ("entry");
}

////////////////////////////////////////////////////////////////////////////////
// Determine the delta between 'from' and 'to', and apply only those changes to
// 'base'.  All three tasks have the same uuid.
void Daemon::patch (
  Task& base,
  const Task& from,
  const Task& to) const
{
  // Determine the different attribute names between from and to.
  std::vector <std::string> from_atts;
  Task::const_iterator att;
  for (att = from.begin (); att != from.end (); ++att)
    from_atts.push_back (att->first);

  std::vector <std::string> to_atts;
  for (att = to.begin (); att != to.end (); ++att)
    to_atts.push_back (att->first);

  std::vector <std::string> from_only;
  std::vector <std::string> to_only;
  listDiff (from_atts, to_atts, from_only, to_only);

  std::vector <std::string> common_atts;
  listIntersect (from_atts, to_atts, common_atts);

  // The from-only attributes must be deleted from base.
  std::vector <std::string>::iterator i;
  for (i = from_only.begin (); i != from_only.end (); ++i)
  {
    _log->format ("[%d] patch remove %s", _txn_count, i->c_str ());
    base.remove (*i);
  }

  // The to-only attributes must be added to base.
  for (i = to_only.begin (); i != to_only.end (); ++i)
  {
    _log->format ("[%d] patch add %s=%s", _txn_count, i->c_str (), to.get (*i).c_str ());
    base.set (*i, to.get (*i));
  }

  // The intersecting attributes, if the values differ, are applied.
  for (i = common_atts.begin (); i != common_atts.end (); ++i)
    if (from.get (*i) != to.get (*i))
    {
      _log->format ("[%d] patch modify %s=%s", _txn_count, i->c_str (), to.get (*i).c_str ());
      base.set (*i, to.get (*i));
    }
}

////////////////////////////////////////////////////////////////////////////////
int command_server (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  bool daemon      = false;
  std::string root = "";

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--daemon", *i, 3)) daemon  = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (taskd_applyOverride (config, *i))   ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";
  }

  if (root == "")
  {
    char* root_env = getenv ("TASKDDATA");
    if (root_env)
      root = root_env;
  }

  // Verify that root exists.
  if (root == "")
    throw std::string ("ERROR: The '--data' option is required.");

  Directory root_dir (root);
  if (!root_dir.exists ())
    throw std::string ("ERROR: The '--data' path does not exist.");

  // Load the config file.
  config.load (root_dir._data + "/config");
  config.set ("root", root_dir._data);

  // Preserve the verbose setting for this run.
  config.set ("verbose", verbose);
  config.set ("debug", debug);

  Log log;

  try
  {
    log.setFile (config.get ("log"));
    log.write (std::string ("==== ")
               + PACKAGE_STRING
               + " "
#ifdef HAVE_COMMIT
               + COMMIT
#endif
               + " ===="
              );

    log.format ("Serving from %s", root.c_str ());

    if (debug)
      log.write ("Debug mode");

    std::string serverDetails = config.get ("server");
    std::string::size_type colon = serverDetails.find (':');

    if (colon == std::string::npos)
      throw std::string ("ERROR: Malformed configuration setting 'server'");

    std::string port = serverDetails.substr (colon + 1);

    // Create a taskd server object.
    Daemon server        (config);
    server.setLog        (&log);
    server._db.setLog    (&log);
    server.setConfig     (&config);
    server.setPort       (port);
    server.setQueueSize  (config.getInteger ("queue.size"));
    server.setLimit      (config.getInteger ("request.limit"));
    server.setLogClients (config.getBoolean ("ip.log"));

    // Optional daemonization.
    if (daemon)
    {
      server.setDaemon   ();
      server.setPidFile  (config.get ("pid.file"));
    }

    // It just runs until you kill it.
    File cert (config.get ("server.cert"));
    server.setCertFile (cert._data);

    File key (config.get ("server.key"));
    server.setKeyFile (key._data);

    server.beginServer ();
  }

  catch (std::string& error)
  {
    log.write (error);
    return -1;
  }

  catch (...)
  {
    log.write ("Unknown error");
    return -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
