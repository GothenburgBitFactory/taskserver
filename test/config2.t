#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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

class TestConfig(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()
        self.td('init --data {0}'.format(self.td.datadir))

    def test_config(self):
        """taskd config --data $TASKDDATA"""
        code, out, err = self.td('config --data {0}'.format(self.td.datadir))
        self.assertIn("request.limit", out)

    def test_config_name_value(self):
        """taskd config --data $TASKDDATA --force name value"""
        code, out, err = self.td('config --data {0} --force name value'.format(self.td.datadir))
        self.assertIn("Config file {0}/config modified.".format(self.td.datadir), out)

    def test_config_name_blank(self):
        """taskd config --data $TASKDDATA --force name ''"""
        code, out, err = self.td('config --data {0} --force name \'\''.format(self.td.datadir))
        self.assertIn("Config file {0}/config modified.".format(self.td.datadir), out)

    def test_config_name(self):
        """taskd config --data $TASKDDATA --force name"""
        self.td('config --data {0} --force name value'.format(self.td.datadir))
        code, out, err = self.td('config --data {0} --force name'.format(self.td.datadir))
        self.assertIn("Config file {0}/config modified.".format(self.td.datadir), out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())
