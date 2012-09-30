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
#include <sstream>
#include <cstring>
#include <inttypes.h>
#include <unistd.h>
#include <Server.h>
#include <Timer.h>
#include <Log.h>
#include <Date.h>
#include <Duration.h>
#include <Color.h>
#include <cmake.h>
#include <commit.h>
#include <text.h>
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
  Config& _config;
  Date _start;
  long _txn_count;
  long _error_count;
  double _busy;
  double _max_time;
  long _bytes_in;
  long _bytes_out;
  std::map <int, std::string> _errors;
};

////////////////////////////////////////////////////////////////////////////////
Daemon::Daemon (Config& settings)
: _config (settings)
, _start (Date ())
, _txn_count (0)
, _error_count (0)
, _busy (0.0)
, _max_time (0.0)
, _bytes_in (0)
, _bytes_out (0)
{
  _errors[200] = "Ok";
  _errors[201] = "No change";
  _errors[202] = "Decline";

  //_errors[300] = "Deprecated request type";
  //_errors[301] = "Redirect";
  //_errors[302] = "Retry";

  //_errors[401] = "Failure";
  //_errors[400] = "Malformed data";
  _errors[401] = "Unsupported encoding";
  //_errors[420] = "Server temporarily unavailable";
  //_errors[430] = "Access denied";
  //_errors[431] = "Account suspended";
  //_errors[432] = "Account terminated";

  _errors[500] = "Syntax error in request";
  //_errors[501] = "Syntax error, illegal parameters";
  _errors[502] = "Not implemented";
  //_errors[503] = "Command parameter not implemented";
  _errors[504] = "Request too big";
  _errors[531] = "Account terminated";
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
    err.set ("status", _errors[e]);
    output = err.serialize ();

    if (_log)
      _log->format ("[%d] ERROR %d %s", _txn_count, e, _errors[e].c_str ());
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
  if (! taskd_authenticate (_config, *_log, in, out))
    return;

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
  int tps                  = 0;
  if (_txn_count)
  {
    average_req       = _bytes_in  / _txn_count;
    average_resp      = _bytes_out / _txn_count;
    average_resp_time = _busy      / _txn_count;

    // Only calculate tps if average_resp_time is non-trivial.
    if (average_resp_time > 0.000001)
      tps = (int) (1.0 / average_resp_time);
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
  out.set ("status",                       _errors[200]);
}

////////////////////////////////////////////////////////////////////////////////
// Sync request.
void Daemon::handle_sync (const Msg& in, Msg& out)
{
  if (! taskd_authenticate (_config, *_log, in, out))
    return;

  if (_log)
    _log->format ("[%d] 'sync' from %s:%d",
                  _txn_count,
                  _client_address.c_str (),
                  _client_port);

  // TODO Logic goes here.
}

////////////////////////////////////////////////////////////////////////////////
int command_server (Config& config, const std::vector <std::string>& args)
{
  int status = 0;

  // Standard argument processing.
  bool verbose     = true;
  bool debug       = false;
  bool daemon      = false;
  bool use_ssl     = false;
  std::string root = "";

  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--quiet",  *i, 3)) verbose = false;
    else if (closeEnough ("--debug",  *i, 3)) debug   = true;
    else if (closeEnough ("--daemon", *i, 3)) daemon  = true;
    else if (closeEnough ("--data",   *i, 3)) root    = *(++i);
    else if (closeEnough ("--ssl",    *i, 3)) use_ssl = true;
    else if (closeEnough ("--nossl",  *i, 3)) use_ssl = false;
    else if (taskd_applyOverride (config, *i))   ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";
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

    if (debug)
      log.write ("Debug mode");

    std::string serverDetails = config.get ("server");
    std::string::size_type colon = serverDetails.find (':');

    if (colon == std::string::npos)
      throw std::string ("ERROR: Malformed configuration setting 'server'");

    int port = strtoimax (serverDetails.substr (colon + 1).c_str (), NULL, 10);

    // Create a taskd server object.
    Daemon server        (config);
    server.setLog        (&log);
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
    if (use_ssl)
    {
/*
      // Resolve paths that may include ~.
      File cert (config.get ("certificate_file"));
      server.setCertFile (cert._data);

      File key (config.get ("key_file"));
      server.setKeyFile (key._data);

      server.beginSSLServer ();
*/
      std::cout << "ERROR: SSL not implemented.\n";
    }
    else
    {
      server.beginServer ();
    }
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
