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
#include <new>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#ifdef CYGWIN
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <Directory.h>
#include <Color.h>
#include <Database.h>
#include <text.h>
#include <taskd.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif
#include <i18n.h>

#ifdef HAVE_LIBGNUTLS
#include <gnutls/gnutls.h>
#endif

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  int status = 0;

  // Create a vector of args.
  std::vector <std::string> args;
  for (int i = 1; i < argc; ++i)
    args.push_back (argv[i]);

  Config config;
  config.set ("verbose", "1");

  // Some options are hard-coded.
  if (args.size ())
  {
    if (args[0] == "-h" || closeEnough ("--help", args[0], 3))
      command_help (args);

    else if (args[0] == "-v" || closeEnough ("--version", args[0], 3))
    {
      Color bold ("bold");
      std::cout << "\n"
                << bold.colorize (PACKAGE_STRING)
#ifdef HAVE_COMMIT
                << " "
                << COMMIT
#endif
                << " built for "

#if defined (DARWIN)
                << "darwin"
#elif defined (SOLARIS)
                << "solaris"
#elif defined (CYGWIN)
                << "cygwin"
#elif defined (HAIKU)
                << "haiku"
#elif defined (OPENBSD)
                << "openbsd"
#elif defined (FREEBSD)
                << "freebsd"
#elif defined (NETBSD)
                << "netbsd"
#elif defined (LINUX)
                << "linux"
#elif defined (KFREEBSD)
                << "GNU/kFreeBSD"
#elif defined (GNUHURD)
                << "GNU/Hurd"
#else
                << "unknown"
#endif

          << "\n"
          << "Copyright (C) 2010 - 2015 Göteborg Bit Factory."
          << "\n"
          << "\n"
          << "Taskd may be copied only under the terms of the MIT license, "
          << "which may be found in the taskd source kit."
          << "\n"
          << "Documentation for taskd can be found using 'man taskd' or at "
          << "http://taskwarrior.org"
          << "\n"
          << "\n";
    }
    else
    {
      try
      {
        // Some defaults come from the environment.
        char* root_env = getenv ("TASKDDATA");
        if (root_env)
          config.set ("root", root_env);

        // Process all the options.
        std::vector <std::string> positionals;
        std::vector <std::string>::iterator arg;
        for (arg = args.begin (); arg != args.end (); ++arg)
        {
          if (closeEnough ("--data", *arg, 3))
          {
            ++arg;
            if (arg == args.end () || (*arg)[0] == '-')
              throw std::string (STRING_TASKD_DATA);

            config.set ("root", *arg);
          }
          else if (closeEnough ("--quiet",  *arg, 3))  config.set ("verbose",      0);
          else if (closeEnough ("--debug",  *arg, 3))  config.set ("debug",        1);
          else if (closeEnough ("--force",  *arg, 3))  config.set ("confirmation", 0);
          else if (closeEnough ("--daemon", *arg, 3))  config.set ("daemon",       1);
          else if (taskd_applyOverride (config, *arg)) ;
          else                                         positionals.push_back (*arg);
        }

        // A database object interfaces to the data.
        Database db (&config);

        // The highest-level commands are hard-coded:
             if (closeEnough ("init",        args[0], 3)) command_init     (db, positionals);
        else if (closeEnough ("config",      args[0], 3)) command_config   (db, positionals);
        else if (closeEnough ("status",      args[0], 3)) command_status   (db, positionals);
        else if (closeEnough ("help",        args[0], 3)) command_help     (    positionals);
        else if (closeEnough ("diagnostics", args[0], 3)) command_diag     (db, positionals);
        else if (closeEnough ("server",      args[0], 3)) command_server   (db, positionals);
        else if (closeEnough ("add",         args[0], 3)) command_add      (db, positionals);
        else if (closeEnough ("remove",      args[0], 3)) command_remove   (db, positionals);
        else if (closeEnough ("suspend",     args[0], 3)) command_suspend  (db, positionals);
        else if (closeEnough ("resume",      args[0], 3)) command_resume   (db, positionals);
        else if (closeEnough ("client",      args[0], 3)) command_client   (db, positionals);
        else if (closeEnough ("validate",    args[0], 3)) command_validate (db, positionals);
        else
          throw format (STRING_TASKD_BAD_COMMAND, args[0]);
      }

      catch (std::string& error)
      {
        if (error == "usage")
        {
          std::vector <std::string> no_args;
          command_help (no_args);
        }
        else
          std::cout << error << "\n";
        status = -1;
      }

      catch (std::bad_alloc& error)
      {
        std::cerr << "Error: Memory allocation failed: " << error.what () << "\n";
        status = -3;
      }

      catch (...)
      {
        std::cerr << STRING_ERROR_UNKNOWN
                  << "\n";
        status = -2;
      }
    }
  }
  else
    command_help (args);

  return status;
}

////////////////////////////////////////////////////////////////////////////////
