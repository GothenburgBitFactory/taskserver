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

#ifndef INCLUDED_TASKD
#define INCLUDED_TASKD

#include <vector>
#include <string>
#include <ConfigFile.h>
#include <Msg.h>
#include <Log.h>
#include <Directory.h>

int command_init      (Config&, const std::vector <std::string>&);
int command_config    (Config&, const std::vector <std::string>&);
int command_status    (Config&, const std::vector <std::string>&);
int command_help      (Config&, const std::vector <std::string>&);
int command_server    (Config&, const std::vector <std::string>&);
int command_add       (Config&, const std::vector <std::string>&);
int command_remove    (Config&, const std::vector <std::string>&);
int command_suspend   (Config&, const std::vector <std::string>&);
int command_resume    (Config&, const std::vector <std::string>&);

// api.cpp
bool taskd_applyOverride (Config&, const std::string&);
int taskd_execute (const std::string&, std::string&);
int taskd_runExtension (const std::string&, const std::string&, Config&, bool);
int taskd_runExtension (const std::string&, const std::string&, std::string&, Config&);
int taskd_runHook (const std::string&, const std::string&, Log&, Config&);

void taskd_requireSetting (Config&, const std::string&);
void taskd_requireVersion (const Msg&, const std::string&);
bool taskd_at_least (const std::string&, const std::string&);
bool taskd_createDirectory (Directory&, bool);

bool taskd_sendMessage (Config&, const std::string&, const Msg&, bool spool = true);
bool taskd_sendMessage (Config&, const std::string&, const Msg&, Msg&, bool spool = true);
void taskd_spoolMessage (Config&, const std::string&, const Msg&);
bool taskd_resendMessage (Config&, const std::string&);
void taskd_renderMap (const std::map <std::string, std::string>&, const std::string&, const std::string&);
bool taskd_allow (const std::string&, const std::vector <std::string>&, const std::vector <std::string>&, Log&, Config&);
bool taskd_match (const Msg&, const Msg&, Log&, Config&);

#endif
////////////////////////////////////////////////////////////////////////////////
