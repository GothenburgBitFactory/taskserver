#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

class TestAddOrg(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_init(self):
        """taskd init --data $TASKDDATA"""
        code, out, err = self.td('init --data {0}'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertTrue(os.path.exists(self.td.datadir))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs')))

    def test_add_org(self):
        """taskd add --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

    def test_add_org_add_org(self):
        """taskd add --data $TASKDDATA org ORG; taskd add --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td.runError('add --data {0} org ORG'.format(self.td.datadir))
        self.assertIn("ERROR: Organization 'ORG' already exists.", out)

    def test_add_org_with_spaces(self):
        """taskd add --data $TASKDDATA org 'MY ORG'"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td("add --data {0} org 'MY ORG'".format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'MY ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'MY ORG', 'users')))

class TestRemoveOrg(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_remove_org(self):
        """taskd remove --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

        code, out, err = self.td('remove --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Removed organization 'ORG'", out)
        self.assertFalse(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

    def test_remove_org_missing(self):
        """taskd remove --data $TASKDDATA org NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td.runError('remove --data {0} org NOPE'.format(self.td.datadir))
        self.assertNotIn("ERROR: Organization 'NOPE' does not exist.", err)

class TestSuspendOrg(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_suspend_org(self):
        """taskd suspend --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

        code, out, err = self.td('suspend --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Suspended organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'suspended')))

    def test_suspend_org_missing(self):
        """taskd suspend --data $TASKDDATA org NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td.runError('suspend --data {0} org ORG'.format(self.td.datadir))
        self.assertIn("ERROR: Organization 'ORG' does not exist.", out)

    def test_suspend_org_suspended(self):
        """taskd suspend --data $TASKDDATA org ORG; taskd suspend --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

        code, out, err = self.td('suspend --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Suspended organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'suspended')))

        code, out, err = self.td.runError('suspend --data {0} org ORG'.format(self.td.datadir))
        self.assertIn("Already suspended.", out)

class TestResumeOrg(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_resume_org(self):
        """taskd resume --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

        code, out, err = self.td('suspend --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Suspended organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'suspended')))

        code, out, err = self.td('resume --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Resumed organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))
        self.assertFalse(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'suspended')))

    def test_resume_missing(self):
        """taskd resume --data $TASKDDATA org NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td.runError('resume --data {0} org NOPE'.format(self.td.datadir))
        self.assertIn("ERROR: Organization 'NOPE' does not exist.", out)

    def test_resume_unsuspended(self):
        """taskd resume --data $TASKDDATA org ORG; taskd resume --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)
        self.assertIn("Created organization 'ORG'", out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users')))

        code, out, err = self.td.runError('resume --data {0} org ORG'.format(self.td.datadir))
        self.assertIn("Not suspended.", out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

