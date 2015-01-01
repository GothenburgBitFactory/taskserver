#! /usr/bin/env python2.7
# coding=UTF-8
################################################################################
## taskd = Taskserver
##
## Copyright 2010 - 2015, Göteborg Bit Factory.
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

import os
import re
import argparse
import datetime

timestamp       = re.compile('^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})')                                      
bounce          = re.compile('==== taskd')
sync1           = re.compile("\'sync\' from ([^/]+)/(.+) at")
sync2           = re.compile("\'sync\' from '([^/]+)/(.+)' using '([^']+)' at")
sync_trivial    = re.compile('still valid')
sync_nontrivial = re.compile('New sync key')
loaded          = re.compile('Loaded (\d+)')
merged          = re.compile('merged (\d+)')
error           = re.compile('ERROR (\d+)')
error_internal  = re.compile('ERROR')
warning         = re.compile('WARNING')

################################################################################
def scan_log(file, data):
  if not os.path.exists(file):
    raise Exception("File '%s' does not exist" % file)

  with open(file) as fh:
    for line in fh.readlines():

      # Capture every timestamp seen.                                                                    
      matches = timestamp.match(line)                                                                             
      if matches:                                                                                                 
        dt = datetime.datetime.strptime(matches.group(1), '%Y-%m-%d %H:%M:%S')
        if data['oldest'] == 0:
          data['oldest'] = dt
        else:
          data['newest'] = dt

      matches = bounce.search(line)
      if matches:
        data['bounce'] += 1

      matches = sync2.search(line)
      if matches:
        data['sync'] += 1
        org    = matches.group(1)
        user   = matches.group(1)+'/'+matches.group(2)
        client = matches.group(3)

        if org in data['active_orgs']:
          data['active_orgs'][org] += 1
        else:
          data['active_orgs'][org] = 1

        if user in data['active_users']:
          data['active_users'][user] += 1
        else:
          data['active_users'][user] = 1

        if client not in data['clients']:
          data['clients'].append(client)
      else:
        matches = sync1.search(line)
        if matches:
          data['sync'] += 1
          org    = matches.group(1)
          user   = matches.group(1)+'/'+matches.group(2)

          if org in data['active_orgs']:
            data['active_orgs'][org] += 1
          else:
            data['active_orgs'][org] = 1

          if user in data['active_users']:
            data['active_users'][user] += 1
          else:
            data['active_users'][user] = 1

      matches = sync_trivial.search(line)
      if matches:
        data['sync_trivial'] += 1

      matches = sync_nontrivial.search(line)
      if matches:
        data['sync_nontrivial'] += 1

      matches = loaded.search(line)
      if matches:
        data['loaded'] += int(matches.group(1))

      matches = merged.search(line)
      if matches:
        data['merged'] += int(matches.group(1))

      matches = error.search(line)
      if matches:
        data['errors'] += 1
        code    = matches.group(1)
        if code in data['errorcode']:
          data['errorcode'][code] += 1
        else:
          data['errorcode'][code] = 1
      else:
        matches = error_internal.search(line)
        if matches:
          data['errors'] += 1

      matches = warning.search(line)
      if matches:
        data['warnings'] += 1

################################################################################
def scan_root(root, data):
  if not os.path.exists(root):
    raise Exception("Directory '%s' does not exist" % root)

  orgs_path = os.path.join(root, 'orgs')
  for org in os.listdir(orgs_path):
    data['total_orgs'].append(org)
    users_path = os.path.join(orgs_path, org, 'users')
    for user in os.listdir(users_path):
      data['total_users'].append(org+'/'+user)
      data_path = os.path.join(users_path, user, 'tx.data')
      if os.path.exists(data_path):
        data['bytes'] += os.stat(data_path).st_size

################################################################################
def show_profile(root, data):
  days = (data['newest'] - data['oldest']).total_seconds() / 86400.0
  total_users = len(data['total_users'])

  print
  print "[1mServer[0m"
  print "  Time range:             ", (data['newest'] - data['oldest'])
  print "  Bounces:                ", data['bounce']
  print "  Average uptime:          %.2f days" % (days / data['bounce'])
  print "  Errors:                 ", data['errors']
  print "  Warnings:               ", data['warnings']
  if root:
    print "  Data stored:            ", data['bytes'], 'bytes'

  if len(data['errorcode']):
    print "  Error Codes"
    for code, count in sorted(data['errorcode'].iteritems()):
      print "    Error %3s         %6d" % (code, count)

  print "[1mConfiguration[0m"
  if root:
    print "  Organizations:          ", len(data['total_orgs'])
    print "  Users:                  ", len(data['total_users'])
  print "  Active Organizations:   ", len(data['active_orgs'])
  print "  Active Users:           ", len(data['active_users'])

  print "[1mTraffic[0m"
  if data['bounce']:
    print "  Syncs per bounce:        %.2f" % (1.0 * data['sync'] / data['bounce'])
  print "  Average syncs:           %.2f per day" % (data['sync'] / days)
  print "  Merged:                  %d taskѕ" % data['merged']
  print "  Loaded:                  %d taskѕ" % data['loaded']
  print "  Clients:                ", len(data['clients'])

  print "[1mUser Profile[0m"
  if root and total_users:
    print "  Syncs:                   %.2f per user, per day" % (data['sync']            / (total_users * days))
    print "  Non-trivial syncs:       %.2f per user, per day" % (data['sync_nontrivial'] / (total_users * days))
  if root and total_users:
    print "  Data:                    %d bytes per user" % (data['bytes'] / total_users)
  if data['sync']:
    print "  Non-trivial sync ratio:  %.2f" % (1.0 * data['sync_nontrivial'] / data['sync'])
  print

  if data['total_orgs']:
    print "[1mOrgs[0m"
    for org in sorted(data['total_orgs']):
      if org in data['active_orgs']:
        print ' ', org
      else:
        print "  %s (inactive)" % org
    print
  else:
    print "[1mOrgs[0m"
    for org in sorted(data['active_orgs']):
      print ' ', org
    print

  if data['total_users']:
    print "[1mUsers[0m"
    for user in sorted(data['total_users']):
      if user in data['active_users']:
        print ' ', user
      else:
        print "  %s (inactive)" % user
    print
  else:
    print "[1mUsers[0m"
    for user in sorted(data['active_users']):
      print ' ', user
    print

  if data['clients']:
    print "[1mClients[0m"
    for client in sorted(data['clients']):
      print ' ', client
    print

################################################################################
def main(args):
  data = { 'bounce'          : 0,
           'sync'            : 0,
           'sync_trivial'    : 0,
           'sync_nontrivial' : 0,
           'active_orgs'     : {},
           'active_users'    : {},
           'total_orgs'      : [],
           'total_users'     : [],
           'clients'         : [],
           'loaded'          : 0,
           'merged'          : 0,
           'oldest'          : 0,
           'newest'          : 0,
           'bytes'           : 0,
           'errors'          : 0,
           'errorcode'       : {},
           'warnings'        : 0 }

  for log in args.log:
    log_file = log[0]
    scan_log(log_file, data)

  if args.data:
    scan_root(args.data, data)

  show_profile(args.data, data)

################################################################################
if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Generate user profiles from server logs.")
  parser.add_argument('log', action='append', help='Log file to scan.', nargs='+')
  parser.add_argument('--data', help='Location of data root.')
  args = parser.parse_args()

  try:
    main(args)
  except Exception as msg:
    print 'Error:', msg

################################################################################
