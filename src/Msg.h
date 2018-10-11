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

#ifndef INCLUDED_MSG
#define INCLUDED_MSG

#include <map>
#include <vector>
#include <string>

class Msg
{
public:
  Msg ();
  Msg (const Msg&);
  Msg& operator= (const Msg&);
  bool operator== (const Msg&) const;
  ~Msg ();

  void clear ();

  void set (const std::string&, const int);
  void set (const std::string&, const std::string&);
  void set (const std::string&, const double);
  void setPayload (const std::string&);
  std::string get (const std::string&) const;
  std::string getPayload () const;

  void all (std::vector <std::string>&) const;
  std::string serialize () const;
  bool parse (const std::string&);

private:
  std::map <std::string, std::string> _header;
  std::string _payload;
};

#endif

