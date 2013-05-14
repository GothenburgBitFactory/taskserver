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
#include <cstring>
#include <stdlib.h>
#include <Date.h>
#include <Color.h>
#include <Timer.h>
#include <cmake.h>
#include <text.h>
#include <util.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
int status_statistics (Config& config)
{
  int status = 0;

  // Request statistics, but do not spool request, in the event of failure.
  Msg request;
  request.set ("type", "statistics");
  request.set ("time", Date ().toISO ());

  Msg response;
  if (taskd_sendMessage (config, "server", request, response))
  {
    std::vector <std::string> names;
    response.all (names);

    std::map <std::string, std::string> data;
    data["Uptime"]                = formatTime (strtol (response.get ("uptime").c_str (), NULL, 10));
    data["Transactions"]          = commify (response.get ("transactions"));
    data["TPS"]                   = format (strtod (response.get ("tps").c_str (), NULL), 4, 5);
    data["Errors"]                = commify (response.get ("errors"));
    data["Idle"]                  = format (100.0 * strtod (response.get ("idle").c_str (), NULL), 3, 5) + "%";
    data["Total In"]              = formatBytes (strtol (response.get ("total bytes in").c_str (), NULL, 10));
    data["Total Out"]             = formatBytes (strtol (response.get ("total bytes out").c_str (), NULL, 10));
    data["Average Request"]       = formatBytes (strtol (response.get ("average request bytes").c_str (), NULL, 10));
    data["Average Response"]      = formatBytes (strtol (response.get ("average response bytes").c_str (), NULL, 10));
    data["Average Response Time"] = response.get ("average response time") + " sec";
    data["Maximum Response Time"] = response.get ("maximum response time") + " sec";

    taskd_renderMap (data, "Tracked Statistic", "Data");
  }
  else
  {
    std::cout << Color ("red").colorize ("ERROR: Task server not responding.") << "\n";
    status = 1;
  }

  return status;
}
 
////////////////////////////////////////////////////////////////////////////////
int command_status (Config& config, const std::vector <std::string>& args)
{
//  taskd_requireConfiguration (config);
//  taskd_resume (config);

  bool verbose = true;
  std::string root;
  std::vector <std::string>::const_iterator i;
  for (i = ++(args.begin ()); i != args.end (); ++i)
  {
         if (closeEnough ("--data",  *i, 3))   root     = *(++i);
    else if (closeEnough ("--quiet", *i, 3))   verbose  = false;
    else if (taskd_applyOverride (config, *i)) ;
    else
      throw std::string ("ERROR: Unrecognized argument '") + *i + "'";
  }

  // Preserve the verbose setting for this run.
  config.set ("verbose", verbose);

  return status_statistics (config);
}

////////////////////////////////////////////////////////////////////////////////
