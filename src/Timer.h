////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TIMER
#define INCLUDED_TIMER

#include <string>
#include <sys/time.h>

// Timer is a scope-activated timer that dumps to std::cout at end of scope.
class Timer
{
public:
  Timer ();
  Timer (const std::string&);
  ~Timer ();
  Timer (const Timer&);
  Timer& operator= (const Timer&);

  void start ();
  void stop ();
  unsigned long total () const;
  void subtract (unsigned long);

  static unsigned long now ();

private:
  std::string    _description;
  bool           _running;
  struct timeval _start;
  unsigned long  _total;
};

// HighResTimer is a stop watch with microsecond resolution.
class HighResTimer
{
public:
  HighResTimer ();
  ~HighResTimer ();
  HighResTimer (const HighResTimer&);
  HighResTimer& operator= (const HighResTimer&);

  void start ();
  void stop ();
  double total () const;

private:
  struct timeval _start;
  struct timeval _stop;
};


#endif

////////////////////////////////////////////////////////////////////////////////
