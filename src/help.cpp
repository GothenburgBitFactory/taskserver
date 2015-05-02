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
#include <cstring>
#include <text.h>
#include <taskd.h>

////////////////////////////////////////////////////////////////////////////////
void command_help (const std::vector <std::string>& args)
{
  if (args.size () > 1)
  {
    if (closeEnough ("init", args[1], 3))
    {
      std::cout << "\n"
                << "taskd init [options]\n"
                << "\n"
                << "Initializes a server instance at <root>.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Debug mode generates lots of diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("config", args[1], 3))
    {
      std::cout << "\n"
                << "taskd config [options] [<name> [<value>]]\n"
                << "\n"
                << "Displays or modifies a configuration variable value.\n"
                << "\n"
                << "  taskd config\n"
                << "      Shows all configuration settings.\n"
                << "\n"
                << "  taskd config <name>\n"
                << "      Deletes the value for <name>.\n"
                << "\n"
                << "  taskd config <name> ''\n"
                << "      Sets <name> to blank.\n"
                << "\n"
                << "  taskd config <name> <value>\n"
                << "      Sets <name> to <value>.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Debug mode generates lots of diagnostics\n"
                << "  --force        Do not ask for confirmation of changes\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("server", args[1], 3))
    {
      std::cout << "\n"
                << "taskd server [options]\n"
                << "\n"
                << "Runs the server.  Requires that the location of the data "
                << "is specified:\n"
                << "  --data         Specifies data location\n"
                << "\n"
                << "Options:\n"
                << "  --daemon       Runs server as a daemon\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Debug mode generates lots of diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n"
                << "Note that sending the HUP signal to the taskd server causes a configuration\n"
                << "file reload before the next request is handled.\n"
                << "\n";
    }
    else if (closeEnough ("status", args[1], 3))
    {
      std::cout << "\n"
                << "Taskserver 'status' is not implemented.\n"
                << "\n";
/*
      std::cout << "\n"
                << "taskd status [options]\n"
                << "\n"
                << "Shows server status.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
*/
    }
#ifdef FEATURE_CLIENT_INTERFACE
    else if (closeEnough ("client", args[1], 3))
    {
      std::cout << "\n"
                << "taskd client [options] <host:port> <file> [<file> ...]\n"
                << "\n"
                << "Sends <file> to Taskserver on <host:port> and displays "
                << "the response.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
#endif
    else if (closeEnough ("add", args[1], 3))
    {
      std::cout << "\n"
                << "taskd add [options] org <org>\n"
                << "taskd add [options] group <org> <group>\n"
                << "taskd add [options] user <org> <user>\n"
                << "\n"
                << "Creates a new organization, group or user.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("remove", args[1], 3))
    {
      std::cout << "\n"
                << "taskd remove [options] org <org>\n"
                << "taskd remove [options] group <org> <group>\n"
                << "taskd remove [options] user <org> <password>\n"
                << "\n"
                << "Deletes an organization, group or user.  Permanently.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("suspend", args[1], 3))
    {
      std::cout << "\n"
                << "taskd suspend [options] org <org>\n"
                << "taskd suspend [options] group <org> <group>\n"
                << "taskd suspend [options] user <org> <password>\n"
                << "\n"
                << "Suspends an organization, group or user.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("resume", args[1], 3))
    {
      std::cout << "\n"
                << "taskd resume [options] org <org>\n"
                << "taskd resume [options] group <org> <group>\n"
                << "taskd resume [options] user <org> <password>\n"
                << "\n"
                << "Resumes, or un-suspends an organization, group or user.\n"
                << "\n"
                << "Options:\n"
                << "  --quiet        Turns off verbose output\n"
                << "  --debug        Generates debugging diagnostics\n"
                << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
                << "  --NAME=VALUE   Temporary configuration override\n"
                << "\n";
    }
    else if (closeEnough ("diag", args[1], 3))
    {
      std::cout << "\n"
                << "taskd diagnostics\n"
                << "\n"
                << "Show installation details.\n"
                << "\n";
    }
    else if (closeEnough ("validate", args[1], 3))
    {
      std::cout << "\n"
                << "taskd validate <JSON | file>\n"
                << "\n"
                << "Run the JSON parser over the provided JSON string or file.\n"
                << "For debugging.\n"
                << "\n";
    }
    else
      std::cout << "No help for '" << args[1] << "'.\n";
  }
  else
    std::cout << "\n"
              << "Usage: taskd -v|--version\n"
              << "       taskd -h|--help\n"
              << "       taskd diagnostics\n"
              << "       taskd validate <JSON | file>\n"
              << "       taskd help [<command>]\n"
              << "\n"
              << "Commands run only on server:\n"
              << "       taskd add     [options] org   <org>\n"
              << "       taskd add     [options] group <org> <group>\n"
              << "       taskd add     [options] user  <org> <user>\n"
              << "       taskd config  [options] [--force] [<name> [<value>]]\n"
              << "       taskd init    [options]\n"
              << "       taskd remove  [options] org   <org>\n"
              << "       taskd remove  [options] group <org> <group>\n"
              << "       taskd remove  [options] user  <org> <user>\n"
              << "       taskd resume  [options] org   <org>\n"
              << "       taskd resume  [options] group <org> <group>\n"
              << "       taskd resume  [options] user  <org> <user>\n"
              << "       taskd server  [options] [--daemon]\n"
              << "       taskd status  [options]\n"
              << "       taskd suspend [options] org   <org>\n"
              << "       taskd suspend [options] group <org> <group>\n"
              << "       taskd suspend [options] user  <org> <user>\n"
              << "\n"
              << "Commands run remotely:\n"
              << "       taskd client  [options] <host:port> <file> [<file> ...]\n"
              << "\n"
              << "Common Options:\n"
              << "  --quiet        Turns off verbose output\n"
              << "  --debug        Generates debugging diagnostics\n"
              << "  --data <root>  Data directory, otherwise $TASKDDATA\n"
              << "  --NAME=VALUE   Temporary configuration override\n"
              << "\n";
}

////////////////////////////////////////////////////////////////////////////////
