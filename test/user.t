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
import re
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

class TestAddUser(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_add_user(self):
        """taskd add --data $TASKDDATA user ORG USER"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

    def test_add_user_add_user(self):
        """taskd add --data $TASKDDATA user ORG USER; taskd add --data $TASKDDATA user ORG USER"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        code, out, err = self.td.runError('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertIn("ERROR: User 'USER' already exists.", err)

    def test_add_user_spaces(self):
        """taskd add --data $TASKDDATA user ORG 'FIRST LAST'"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG \'FIRST LAST\''.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'FIRST LAST\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

class TestRemoveUser(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_remove_user(self):
        """taskd remove --data $TASKDDATA user ORG KEY"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

        code, out, err = self.td('remove --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Removed user \'{0}\' from organization \'ORG\''.format(key), out)
        self.assertFalse(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

    def test_remove_user_missing(self):
        """taskd remove --data $TASKDDATA user ORG NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td.runError('remove --data {0} user ORG NOPE'.format(self.td.datadir))
        self.assertIn('User \'NOPE\' does not exist.', err)

class TestSuspendUser(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_suspend_user(self):
        """taskd suspend --data $TASKDDATA org ORG"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

        code, out, err = self.td('suspend --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Suspended user \'{0}\' in organization \'ORG\''.format(key), out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key, 'suspended')))

    def test_suspend_user_missing(self):
        """taskd suspend --data $TASKDDATA user ORG NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td.runError('suspend --data {0} user ORG NOPE'.format(self.td.datadir))
        self.assertIn("ERROR: User 'NOPE' does not exist.", err)

    def test_suspend_user_suspended(self):
        """taskd suspend --data $TASKDDATA user ORG USER; taskd suspend --data $TASKDDATA user ORG USER"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

        code, out, err = self.td('suspend --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Suspended user \'{0}\' in organization \'ORG\''.format(key), out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key, 'suspended')))

        code, out, err = self.td.runError('suspend --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn("Already suspended.", err)

class TestResumeUser(ServerTestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.td = Taskd()

    def test_resume_user(self):
        """taskd resume --data $TASKDDATA user ORG KEY"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

        code, out, err = self.td('suspend --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Suspended user \'{0}\' in organization \'ORG\''.format(key), out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key, 'suspended')))

        code, out, err = self.td('resume --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Resumed user \'{0}\' in organization \'ORG\''.format(key), out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))
        self.assertFalse(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key, 'suspended')))

    def test_resume_missing(self):
        """taskd resume --data $TASKDDATA user ORG NOPE"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td.runError('resume --data {0} user ORG NOPE'.format(self.td.datadir))
        self.assertIn("ERROR: User 'NOPE' does not exist.", err)

    def test_resume_unsuspended(self):
        """taskd resume --data $TASKDDATA user ORG KEY"""
        self.td('init --data {0}'.format(self.td.datadir))
        self.td('add --data {0} org ORG'.format(self.td.datadir))
        code, out, err = self.td('add --data {0} user ORG USER'.format(self.td.datadir))
        self.assertNotIn("ERROR", err)

        reKey = re.compile ('New user key: ([a-z0-9-]{36})')
        match = reKey.match (out)
        self.assertTrue(match)
        key = match.group(1)

        self.assertIn('Created user \'USER\' for organization \'ORG\'', out)
        self.assertTrue(os.path.exists(os.path.join(self.td.datadir, 'orgs', 'ORG', 'users', key)))

        code, out, err = self.td.runError('resume --data {0} user ORG {1}'.format(self.td.datadir, key))
        self.assertIn('Not suspended.', err)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

