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
#include <stdlib.h>
#include <Date.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <taskd.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
// This is a debugging-only command that uploads a file to the server, and then
// displays the result.
void command_client (Database& db, const std::vector <std::string>& args)
{
#ifdef FEATURE_CLIENT_INTERFACE
  // Parse arguments.
  if (args.size () < 3)
    throw std::string (STRING_CLIENT_USAGE);

  db._config->set ("server", args[1]);

  for (unsigned int i = 2; i < args.size (); ++i)
  {
    // Read file.
    File file (args[i]);
    std::string contents;
    file.read (contents);
    std::cout << ">>> " << args[i] << "\n";

    Msg request;
    request.parse (contents);
    request.set ("time", Date ().toISO ());

    Msg response;
    if (! taskd_sendMessage (*db._config, "server", request, response))
      throw std::string (STRING_SERVER_DOWN);

    std::cout << "<<<\n"
              << response.serialize ();
  }
#else
  throw std::string (STRING_CLIENT_DISABLED);
#endif
}

////////////////////////////////////////////////////////////////////////////////

