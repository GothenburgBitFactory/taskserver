#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Taskd, ServerTestCase


# Test methods available:
#     self.assertEqual(a, b)
#     self.assertNotEqual(a, b)
#     self.assertTrue(x)
#     self.assertFalse(x)
#     self.assertIs(a, b)
#     self.assertIsNot(a, b)
#     self.assertIsNone(x)
#     self.assertIsNotNone(x)
#     self.assertIn(a, b)
#     self.assertNotIn(a, b)
#     self.assertIsInstance(a, b)
#     self.assertNotIsInstance(a, b)
#     self.assertRaises(e)
#     self.assertRegexpMatches(t, r)
#     self.assertNotRegexpMatches(t, r)
#     self.tap("")

class TestInit(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

# Note: These three permissions tests cause problems, and are disabled.
#       Somewhere in libshares/src/FS.cpp ::access fails and the code throws.
#       Additionally, the tests create and un-removable self.td.datadir because
#       of the permissions changes.

#    def test_bad_perms_200(self):
#        """chmod 200 $TASKDDATA; taksd init --data $TASKDDATA"""
#        os.chmod(self.td.datadir, 0o200)
#        code, out, err = self.td.runError('init --data {0}'.format(self.td.datadir))
#        self.assertIn("ERROR: The '--data' directory is not readable.", out)
#        os.chmod(self.td.datadir, 0o700)

#    def test_bad_perms_400(self):
#        """chmod 400 $TASKDDATA; taksd init --data $TASKDDATA"""
#        os.chmod(self.td.datadir, 0o400)
#        code, out, err = self.td.runError('init --data {0}'.format(self.td.datadir))
#        self.assertIn("ERROR: The '--data' directory is not writable.", out)
#        os.chmod(self.td.datadir, 0o700)

#    def test_bad_perms_600(self):
#        """chmod 600 $TASKDDATA; taksd init --data $TASKDDATA"""
#        os.chmod(self.td.datadir, 0o600)
#        code, out, err = self.td.runError('init --data {0}'.format(self.td.datadir))
#        self.assertIn("ERROR: The '--data' directory is not executable.", out)
#        os.chmod(self.td.datadir, 0o700)

    def test_init(self):
        """taskd init --data $TASKDDATA"""
        code, out, err = self.td('init --data {0}'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs')))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

