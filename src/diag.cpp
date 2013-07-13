////////////////////////////////////////////////////////////////////////////////
// taskd - Task Server
//
// Copyright 2012 - 2013, Göteborg Bit Factory.
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
#include <unistd.h>
#include <ConfigFile.h>
#include <Color.h>
#include <Msg.h>
#include <taskd.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

#ifdef HAVE_LIBGNUTLS                                                                                                               
#include <gnutls/gnutls.h>                                                                                                          
#endif

////////////////////////////////////////////////////////////////////////////////
int command_diag (Config& config, const std::vector <std::string>& args)
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
               "<unknown>"
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
            << " +c" << sizeof (char)
            << " +i" << sizeof (int)
            << " +l" << sizeof (long)
            << " +vp" << sizeof (void*)
            << "\n\n";

  std::cout << bold.colorize ("Build Features")
            << "\n"

  // Build date.
            << "       Built: " << __DATE__ << " " << __TIME__ << "\n"
#ifdef HAVE_COMMIT
            << "      Commit: " << COMMIT << "\n"
#endif
            << "       CMake: " << CMAKE_VERSION << "\n"
            << "        Caps:"
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

#ifdef HAVE_LIBGNUTLS
            << " +tls"
#else
            << " -tls"
#endif
            << "\n";

  std::cout << "     libuuid: "
#if defined (HAVE_UUID) and defined (HAVE_UUID_UNPARSE_LOWER)
            << "libuuid + uuid_unparse_lower"
#elif defined (HAVE_UUID) and !defined (HAVE_UUID_UNPARSE_LOWER)
            << "libuuid, no uuid_unparse_lower"
#else
            << "n/a"
#endif
            << "\n";

  std::cout << "   libgnutls: "
#ifdef HAVE_LIBGNUTLS
            << GNUTLS_VERSION
#else
            << "n/a"
#endif
            << "\n\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
