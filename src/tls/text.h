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

#ifndef INCLUDED_TEXT
#define INCLUDED_TEXT

#include <vector>
#include <string>

// text.cpp, Non-UTF-8 aware.
std::string trimLeft (const std::string& in, const std::string& t = " ");
std::string trimRight (const std::string& in, const std::string& t = " ");
std::string trim (const std::string& in, const std::string& t = " ");
const std::string format (char);
const std::string format (int);
const std::string format (float, int, int);
const std::string format (double, int, int);
const std::string format (double);
const std::string format (const std::string&, const std::string&);
const std::string format (const std::string&, int);
const std::string format (const std::string&, const std::string&, const std::string&);
const std::string format (const std::string&, const std::string&, int);
const std::string format (const std::string&, const std::string&, double);
const std::string format (const std::string&, int, const std::string&);
const std::string format (const std::string&, int, int);
const std::string format (const std::string&, int, int, int);
const std::string format (const std::string&, int, double);
const std::string format (const std::string&, const std::string&, const std::string&, const std::string&);

#endif
////////////////////////////////////////////////////////////////////////////////
