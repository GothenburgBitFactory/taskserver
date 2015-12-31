#! /usr/bin/env perl
################################################################################
##
## Copyright 2010 - 2016, Göteborg Bit Factory.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

use strict;
use warnings;
use Test::More tests => 11;

# Check for Cygwin, which does not support the 'chmod 000 $data' command.
my $output = qx{../src/taskd --version 2>&1};
my ($platform) = $output =~ /built for (\S+)/;
diag ("Platform: $platform");

# Check for required --data option.
$output = qx{../src/taskd init 2>&1};
like ($output, qr/^ERROR: The '--data' option is required\./, "'taskd init' - missing --data option");

# Check that --data exists.
my $data = 'init.data';
$output = qx{../src/taskd init --data $data 2>&1};
like ($output, qr/^ERROR: The '--data' path does not exist\./, "'taskd init --data $data' - missing data dir");

qx{touch $data};
$output = qx{../src/taskd init --data $data 2>&1};
like ($output, qr/^ERROR: The '--data' path is not a directory\./, "'taskd init --data $data' - data not a directory");
qx{rm $data; mkdir $data};

if ($platform eq 'cygwin')
{
  pass ("'taskd init --data $data' - data not readable (skipped for $platform)");
  pass ("'taskd init --data $data' - data not writable (skipped for $platform)");
  pass ("'taskd init --data $data' - data not executable (skipped for $platform)");
}
else
{
  if ($> != 0)
  {
    qx{chmod 000 $data};
    $output = qx{../src/taskd init --data $data 2>&1};
    like ($output, qr/^ERROR: The '--data' directory is not readable\./, "'taskd init --data $data' - data not readable");

    qx{chmod +r $data};
    $output = qx{../src/taskd init --data $data 2>&1};
    like ($output, qr/^ERROR: The '--data' directory is not writable\./, "'taskd init --data $data' - data not writable");

    qx{chmod +w $data};
    $output = qx{../src/taskd init --data $data 2>&1};
    like ($output, qr/^ERROR: The '--data' directory is not executable\./, "'taskd init --data $data' - data not executable");
  }
  else
  {
    pass ("'taskd init --data $data' - permissions test skipped by root");
    pass ("'taskd init --data $data' - permissions test skipped by root");
    pass ("'taskd init --data $data' - permissions test skipped by root");
  }
}

qx{chmod +x $data};
$output = qx{../src/taskd init --data $data 2>&1};
unlike ($output, qr/^ERROR/, "'taskd init --data $data' - no errors");
ok (-d $data,                "$data exists and is a directory");
ok (-d $data.'/orgs',        "$data/orgs exists and is a directory");
ok (-f $data.'/config',      "$data/config exists and is a file");

# Cleanup.
qx{rm -rf $data};
ok (! -d $data, "Removed $data");

exit 0;

