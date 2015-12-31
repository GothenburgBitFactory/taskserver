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
use Test::More tests => 13;

# Create an instance.
my $data = 'config.data';
qx{mkdir $data};

my $output = qx{../src/taskd init --data $data 2>&1};
unlike ($output, qr/^ERROR/, "'taskd init --data $data' - no errors");
ok (-d $data,                "$data exists and is a directory");

$output = qx{../src/taskd config --data $data --force name value 2>&1};
unlike ($output, qr/^ERROR/, "'taskd config --data $data --force name value' - no errors");
like   ($output, qr/config modified/, 'name/value modified');

$output = qx{../src/taskd config --data $data 2>&1};
unlike ($output, qr/^ERROR/, "'taskd config --data $data' - no errors");
like   ($output, qr/name\s+value/, 'name/value removed');

$output = qx{../src/taskd config --data $data --force name '' 2>&1};
unlike ($output, qr/^ERROR/, "'taskd config --data $data --force name \'\'' - no errors");
like   ($output, qr/config modified/, 'name/value modified');

$output = qx{../src/taskd config --data $data --force name 2>&1};
unlike ($output, qr/^ERROR/, "'taskd config --data $data --force name' - no errors");
like   ($output, qr/config modified/, 'name/value modified');

$output = qx{../src/taskd config --data $data 2>&1};
unlike ($output, qr/^ERROR/, "'taskd config --data $data' - no errors");
unlike ($output, qr/name\s+value/, 'name/value removed');

# Cleanup.
qx{rm -rf $data};
ok (! -d $data, "Removed $data");

exit 0;

