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
#ifndef INCLUDED_SERVER
#define INCLUDED_SERVER

#include <sys/types.h>
#include <string>
#include <ConfigFile.h>
#include <Log.h>

class Server
{
public:
  Server ();
  virtual ~Server ();
  void setHost (const std::string&);
  void setPort (const std::string&);
  void setFamily (const std::string&);
  void setPoolSize (int);
  void setQueueSize (int);
  void setDaemon ();
  void setBlocking ();
  void setNonBlocking ();
  void setPidFile (const std::string&);
  void setLog (Log*);
  void setConfig (Config*);
  void setLimit (int);
  void setCAFile (const std::string&);
  void setCertFile (const std::string&);
  void setKeyFile (const std::string&);
  void setCRLFile (const std::string&);
  void setLogClients (bool);
  void start ();

  void beginServer ();

  void stats (int&, time_t&, double&);

  virtual void handler (const std::string&, std::string&) = 0;

protected:
  void daemonize ();
  void writePidFile ();
  void removePidFile ();
  Log* _log;
  Config* _config;
  bool _log_clients;
  std::string _client_address;
  int _client_port;

private:
  std::string _host;
  std::string _port;
  std::string _family;
  int _pool_size;
  int _queue_size;
  bool _daemon;
  std::string _pid_file;
  int _request_count;
  int _limit;
  std::string _ca_file;
  std::string _cert_file;
  std::string _key_file;
  std::string _crl_file;
};

#endif

////////////////////////////////////////////////////////////////////////////////

