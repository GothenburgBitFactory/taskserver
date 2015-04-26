////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2012 - 2015, Göteborg Bit Factory.
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
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ConfigFile.h>
#include <Color.h>
#include <Msg.h>
#include <File.h>
#include <Directory.h>
#include <JSON.h>
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
void command_diag (Database& config, const std::vector <std::string>& args)
{
  // No argument processing.

  Color bold ("bold");

  std::cout << "\n"
            << bold.colorize (PACKAGE_STRING)
            << "\n";

  std::cout << "    Platform: "
            <<
#if defined (DARWIN)
               "Darwin"
#elif defined (SOLARIS)
               "Solaris"
#elif defined (CYGWIN)
               "Cygwin"
#elif defined (HAIKU)
               "Haiku"
#elif defined (OPENBSD)
               "OpenBSD"
#elif defined (FREEBSD)
               "FreeBSD"
#elif defined (NETBSD)
               "NetBSD"
#elif defined (LINUX)
               "Linux"
#elif defined (KFREEBSD)
               "GNU/kFreeBSD"
#elif defined (GNUHURD)
               "GNU/Hurd"
#else
               "unknown"
#endif
            << "\n";

  char hostname[128] = {0};
  gethostname (hostname, 128);
  std::cout << "    Hostname: "
            << hostname
            << "\n\n";

  // Compiler.
  std::cout << bold.colorize ("Compiler")
            << "\n"
#ifdef __VERSION__
            << "     Version: "
            << __VERSION__ << "\n"
#endif
            << "        Caps:"
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
            << " +c"      << 8 * sizeof (char)
            << " +i"      << 8 * sizeof (int)
            << " +l"      << 8 * sizeof (long)
            << " +vp"     << 8 * sizeof (void*)
            << " +time_t" << 8 * sizeof (time_t)
            << "\n";

  // Compiler compliance level.
  std::string compliance = "non-compliant";
#ifdef __cplusplus
  int level = __cplusplus;
  if (level == 199711)
    compliance = "C++98/03";
  else if (level == 201103)
    compliance = "C++11";
  else
    compliance = format (level);
#endif
  std::cout << "  Compliance: "
            << compliance
            << "\n\n";

  std::cout << bold.colorize ("Build Features")
            << "\n"

  // Build date.
            << "       Built: " << __DATE__ << " " << __TIME__ << "\n"
#ifdef HAVE_COMMIT
            << "      Commit: " << COMMIT << "\n"
#endif
            << "       CMake: " << CMAKE_VERSION << "\n";

  std::cout << "     libuuid: "
#ifdef HAVE_UUID_UNPARSE_LOWER
            << "libuuid + uuid_unparse_lower"
#else
            << "libuuid, no uuid_unparse_lower"
#endif
            << "\n";

  std::cout << "   libgnutls: "
#ifdef HAVE_LIBGNUTLS
#ifdef GNUTLS_VERSION
            << GNUTLS_VERSION
#elif defined LIBGNUTLS_VERSION
            << LIBGNUTLS_VERSION
#endif
#else
            << "n/a"
#endif
            << "\n";

  std::cout << "  Build type: "
#ifdef CMAKE_BUILD_TYPE
            << CMAKE_BUILD_TYPE
#else
            << "-"
#endif
            << "\n\n";

  // Configuration details, if possible.
  char* root_env = getenv ("TASKDDATA");
  std::cout << bold.colorize ("Configuration")
            << "\n"
            << "   TASKDDATA: " << (root_env ? root_env : "") << "\n";

  // Verify that root exists.
  std::string root = config._config->get ("root");
  if (root == "")
  {
    std::cout << "\nBy specifying the '--data' location, more diagnostics can be generated.\n";
  }
  else
  {
    Directory root_dir (config._config->get ("root"));
    if (! root_dir.exists ())
      std::cout << "  root directory does not exist.\n";
    else
    {
      std::cout << "        root: "
                << root_dir._data << (root_dir.readable ()
                    ? " (readable)"
                    : " (not readable)")
                << "\n";

      File config_file (root_dir._data + "/config");
      std::cout << "      config: "
                << config_file._data << (config_file.readable () ? " (readable)" : " (missing)")
                << "\n";

      if (config_file.readable ())
      {
        // Load the config file.
        config._config->load (config_file._data);

        File ca_cert (config._config->get ("ca.cert"));
        File server_cert (config._config->get ("server.cert"));
        File server_key (config._config->get ("server.key"));
        File server_crl (config._config->get ("server.crl"));
        File client_cert (config._config->get ("client.cert"));
        File client_key (config._config->get ("client.key"));

        std::cout << "          CA: "
                  << ca_cert._data << (ca_cert.readable () ? " (readable)" : "")
                  << "\n";
        std::cout << " Certificate: "
                  << server_cert._data << (server_cert.readable () ? " (readable)" : " (missing)")
                  << "\n";
        std::cout << "         Key: "
                  << server_key._data << (server_key.readable () ? " (readable)" : " (missing)")
                  << "\n";
        std::cout << "         CRL: "
                  << server_crl._data << (server_crl.readable () ? " (readable)" : "")
                  << "\n";

        File log (config._config->get ("log"));
        std::cout << "         Log: "
                  << log._data << (log.exists () ? " (found)" : " (missing)")
                  << "\n";

        File pid (config._config->get ("pid.file"));
        std::cout << "    PID File: "
                  << pid._data << (pid.exists () ? " (found)" : " (missing)")
                  << "\n";

        std::cout << "      Server: " << config._config->get ("server") << "\n";
        std::cout << " Max Request: " << config._config->get ("request.limit") << " bytes\n";
        std::cout << "     Ciphers: " << config._config->get ("ciphers") << "\n";

        // Show trust level.
        std::string trust_value = config._config->get ("trust");
        if (trust_value == "strict" ||
            trust_value == "allow all")
          std::cout << "       Trust: " << trust_value << "\n";
        else
          std::cout << "       Trust: Bad value - see 'man taskdrc'\n";
      }
    }
  }

  std::cout << "\n\n";
}

////////////////////////////////////////////////////////////////////////////////
void command_validate (Database& config, const std::vector <std::string>& args)
{
  try
  {
    if (args.size () < 2)
    {
      std::cout << STRING_JSON_VALIDATE << "\n";
      return;
    }

    json::value* root;
    File file (args[1]);
    if (file.exists ())
    {
      std::string contents;
      file.read (contents);
      root = json::parse (contents);
    }
    else
      root = json::parse (args[1]);

    if (root)
    {
      std::cout << STRING_JSON_SYNTAX_OK
                << "\n\n"
                << root->dump ()
                << "\n";
    }

    delete root;
  }

  catch (const std::string& e) { std::cout << e << "\n";                    }
  catch (...)                  { std::cout << STRING_ERROR_UNKNOWN << "\n"; }
}

////////////////////////////////////////////////////////////////////////////////
