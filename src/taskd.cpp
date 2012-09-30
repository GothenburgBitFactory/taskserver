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
#include <text.h>
#include <taskd.h>
#include <cmake.h>
#include <commit.h>

#ifdef HAVE_SRANDOM
#define srand(x) srandom(x)
#endif

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  // Set up randomness.
#ifdef CYGWIN
  srand (time (NULL));
#else
  struct timeval tv;
  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);
#endif

  int status = 0;

  // Create a vector of args.
  std::vector <std::string> args;
  for (int i = 1; i < argc; ++i)
    args.push_back (argv[i]);

  Config config;
  // TODO Load config file.

  // Some options are hard-coded.
  if (args.size ())
  {
    if (args[0] == "-v" || closeEnough ("--version", args[0], 3))
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
#else
                << "unknown"
#endif

          << "\n"
          << "Copyright (C) 2010 - 2012 Göteborg Bit Factory."
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
    else if (args[0] == "-d" || closeEnough ("--diagnostics", args[0], 3))
    {
      std::cout << "\n[1m" << PACKAGE_STRING << "[0m\n";

      std::cout << "  Platform: "
                <<
#if defined (DARWIN)
                   "Darwin"
#elif defined (SOLARIS)
                   "Solaris"
#elif defined (CYGWIN)
                   "Cygwin"
#elif defined (OPENBSD)
                   "OpenBSD"
#elif defined (HAIKU)
                   "Haiku"
#elif defined (FREEBSD)
                   "FreeBSD"
#elif defined (NETBSD)
                   "netbsd"
#elif defined (LINUX)
                   "Linux"
#else
                   "<unknown>"
#endif
                << "\n\n";

      // Compiler.
      std::cout << "[1mCompiler[0m\n"
#ifdef __VERSION__
                << "   Version: " << __VERSION__ << "\n"
#endif
                << "      Caps:"
#ifdef __STDC__
                << " +stdc"
#endif
#ifdef __STDC_HOSTED__
                << " +stdc_hosted"
#endif
#ifdef __STDC_VERSION__
                << " +" << __STDC_VERSION__
#endif
#ifdef _POSIX_VERSION
                << " +" << _POSIX_VERSION
#endif
#ifdef _POSIX2_C_VERSION
                << " +" << _POSIX2_C_VERSION
#endif
#ifdef _ILP32
                << " +ILP32"
#endif
#ifdef _LP64
                << " +LP64"
#endif
                << " +c" << sizeof (char)
                << " +i" << sizeof (int)
                << " +l" << sizeof (long)
                << " +vp" << sizeof (void*)
                << "\n\n";

      std::cout << "[1mBuild Features[0m\n"
      // Build date.
                << "     Built: " << __DATE__ << " " << __TIME__ << "\n"
#ifdef HAVE_COMMIT
                << "    Commit: " << COMMIT << "\n"
#endif
#ifdef CMAKE_VERSION
                << "     CMake: " << CMAKE_VERSION << "\n"
#endif
                << "      Caps:"
#ifdef HAVE_LIBPTHREAD
                << " +pthreads"
#else
                << " -pthreads"
#endif

#ifdef HAVE_SRANDOM
                << " +srandom"
#else
                << " -srandom"
#endif

#ifdef HAVE_RANDOM
                << " +random"
#else
                << " -random"
#endif

#ifdef HAVE_UUID
                << " +uuid"
#else
                << " -uuid"
#endif

#ifdef HAVE_OPENSSL
                << " +ssl"
#else
                << " -ssl"
#endif
                << "\n\n";

      std::cout << "[1mExternal Libraries[0m\n"
#ifdef HAVE_OPENSSL
                << "   OpenSSL: " << OPENSSL_VERSION_TEXT << "\n"
#endif
                << "\n";
    }
    else
    {
      try
      {
        // The highest-level commands are hard-coded:
             if (closeEnough ("init",      args[0], 3)) status = command_init      (config, args);
        else if (closeEnough ("config",    args[0], 3)) status = command_config    (config, args);
        else if (closeEnough ("status",    args[0], 3)) status = command_status    (config, args);
        else if (closeEnough ("help",      args[0], 3)) status = command_help      (config, args);
        else if (closeEnough ("server",    args[0], 3)) status = command_server    (config, args);

        else if (closeEnough ("add",       args[0], 3)) status = command_add       (config, args);
        else if (closeEnough ("remove",    args[0], 3)) status = command_remove    (config, args);
        else if (closeEnough ("suspend",   args[0], 3)) status = command_suspend   (config, args);
        else if (closeEnough ("resume",    args[0], 3)) status = command_resume    (config, args);
        else
        {
          File subcommand (std::string (TASKD_EXTDIR) + "/taskd_" + args[0]);
          if (subcommand.exists () &&
              subcommand.executable ())
          {
            std::string command;
            join (command, " ", args);
            command = std::string (TASKD_EXTDIR) + "/taskd_" + command;

            std::string output;
            status = taskd_execute (command, output);
            std::cout << output;
          }
          else
            throw std::string ("ERROR: Did not recognize command '") + args[0] + "'.";
        }
      }

      catch (std::string& error)
      {
        std::cout << error << "\n";
        status = -1;
      }

      catch (...)
      {
        std::cerr << "Unknown error.\n";
        status = -2;
      }
    }
  }
  else
    status = command_help (config, args);

  return status;
}

////////////////////////////////////////////////////////////////////////////////
