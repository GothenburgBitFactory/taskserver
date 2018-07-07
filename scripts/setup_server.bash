#!/usr/bin/env bash

################################################################################
#
# Copyright 2010 - 2018, Göteborg Bit Factory.
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
################################################################################

# Ask a series of questions, and proceed through a fully automated Taskserver
# configuration and setup. This script has two purposes:
#
# 1. To allow casual users to simply setup a server for taskwarrior users,
#    without needing to read or follow the detailed setup process.
# 2. To provide an example of the setup process for advanced users to modify.

################################################################################
# Ensure spaces are handled as-is in paths read from user
IFS=''

# Useful functions.
log_ok()
{
  echo "[32m$1[0m"
}

log_warning()
{
  echo "[33m$1[0m"
}

log_error()
{
  echo "[31m$1[0m"
}

log_line()
{
  printf "%-60s " "$1"
}

log()
{
  echo "$1"
}

configure()
{
  log_line "$1"
  OUTPUT=$($TASKD_BINARY config --force --data $TASKDDATA "$2" "$3")
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    log
    log $OUTPUT
    exit 1
  fi
}

makedir()
{
  log_line " --> $1"
  if [ -d "$2" ]; then
    log_warning "Already exists"
  else
    PARENT=$(dirname $2)

    if [ -w $PARENT ]; then
      mkdir -p "$2"
      if [ $? -eq 0 ]; then
        log_ok "Ok"
      else
        log_error "Failed"
        log
        log "There was a problem creating $2"
        exit 1
      fi
    fi
  fi
}

setpkipath()
{
    if [ "${OSTYPE:0:6}" = "darwin" ]; then
        local _script_name=$(basename $2)
        local _script_path=$(pwd)/$2
        local _pki_path=${_script_path:0:${#_script_path}-${#_script_name}}$3
        eval "$1=$_pki_path"
    elif [[ "${OSTYPE}" =~ \w*bsd.* ]]; then
        eval "$1=$(realpath $(dirname $2))/$3"
    else
        eval "$1=$(readlink -f $(dirname $2))/$3"
    fi

}

################################################################################
# Taskserver defaults.
DEFAULT_SETUP="$0"
DEFAULT_TASKDDATA="${TASKDDATA:-/var/taskd}"
DEFAULT_HOST=localhost
DEFAULT_PORT=53589
setpkipath DEFAULT_PKI $DEFAULT_SETUP ../pki

# Explain what is going to happen.
log
log_warning "This is an experimental script. Please report any problems."
log
log "Simple Taskserver configuration and setup."
log
log "This script will check your system, then setup and launch a Taskserver"
log "instance for you. You will be asked to confirm before anything is written."
log "Please be careful with your answers and confirm that what the script"
log "collected is correct."
log

# Show setup script location.
log_line "Location of this script"
log_ok $DEFAULT_SETUP

# Look for pki scripts, relative to $0, which assumes this script is in the
# $REPO/scripts/ directory.
log_line "Checking for pki script directory"
if [ -n "$DEFAULT_PKI" -a -d $DEFAULT_PKI ]; then
  log_ok "Found $DEFAULT_PKI"
else
  log_error "error"
  exit 1
fi

# Verify certtool or gnutls-certtool is available.
log_line "Looking for certtool or gnutls-certtool"
CERTTOOL=
FOUND_GNUTLSCERTTOOL=$(which gnutls-certtool 2>/dev/null)
FOUND_CERTTOOL=$(which certtool 2>/dev/null)
if [ -n "$FOUND_GNUTLSCERTTOOL" ]; then
  CERTTOOL=$FOUND_GNUTLSCERTTOOL
  log_ok "Found $FOUND_GNUTLSCERTTOOL"
else
  if [ -n "$FOUND_CERTTOOL" ]; then
    CERTTOOL=$FOUND_CERTTOOL
    log_ok "Found $FOUND_CERTTOOL"
  else
    log_error "Not found"
    exit 1
  fi
fi

# Verify taskd is in $PATH.
log_line "Looking for 'taskd' in \$PATH"
TASKD_BINARY=$(which taskd 2>/dev/null)
if [ -n "$TASKD_BINARY" ]; then
  log_ok "Found $TASKD_BINARY"
else
  log_error "Not found"
  log
  log "Either your \$PATH variable does not include the install location, or"
  log "you did not build and install taskd."
  exit 1
fi

# Verify taskdctl is in $PATH.
log_line "Looking for 'taskdctl' in \$PATH"
TASKDCTL_BINARY=$(which taskdctl 2>/dev/null)
if [ -n "$TASKDCTL_BINARY" ]; then
  log_ok "Found $TASKDCTL_BINARY"
else
  log_error "Not found"
  log
  log "Either your \$PATH variable does not include the install location, or"
  log "you did not build and install taskd."
  exit 1
fi

# Determine $TASKDDATA.
log_line "Choose a directory for all data: [$DEFAULT_TASKDDATA]"
read TASKDDATA
TASKDDATA="${TASKDDATA:-$DEFAULT_TASKDDATA}"
if [ ${TASKDDATA:0:1} != "/" ]; then
  log_line
  log_error "Relative path"
  log
  log "You may not specify a relative path"
  exit 1
fi
log_line " --> Using TASKDDATA"
log_ok $TASKDDATA

# Determine $USER is the desired owner of $TASKDDATA.
log_line "User ID that will run the server [$USER]"
read USER_ID
USER_ID="${USER_ID:-$USER}"
if [ "$USER_ID" == "$USER" ]; then
  log_line " --> Using USER_ID"
  log_ok $USER_ID
else
  USER_EXISTS=$(id $USER_ID 2>&1)
  if [ $? -eq 0 ]; then
    log_line
    log_error "Wrong account"
    log
    log "You are logged in as '$USER'. Log in as '$USER_ID' and rerun $DEFAULT_SETUP"
  else
    log_line
    log_error "Missing user"
    log
    log "The user '$USER_ID' does not yet exist"
  fi
  exit 1
fi

# Determine host name for the server.
log_line "Choose a hostname to listen on: [$DEFAULT_HOST]"
read HOST
HOST="${HOST:-$DEFAULT_HOST}"
log_line " --> Using HOST"
log_ok $HOST

# Determine port number to listen on.
log_line "Choose a port to listen on: [$DEFAULT_PORT]"
read PORT
PORT="${PORT:-$DEFAULT_PORT}"
log_line " --> Using PORT"
log_ok $PORT

# Initial data gathering complete - confirm permission to proceed.
log_line "Okay to proceed and configure server? [y|N]"
read PROCEED
PROCEED="${PROCEED:-N}"
if [ "$PROCEED" != "y" ]; then
  log_error "Terminating - nothing was changed"
  exit 1
fi

# Create $TASKDDATA and subfolders
log "Creating \$TASKDDATA and subfolders"

makedir "\$TASKDDATA" $TASKDDATA
makedir "\$TASKDDATA/cert" $TASKDDATA/cert
makedir "\$TASKDDATA/log" $TASKDDATA/log
makedir "\$TASKDDATA/run" $TASKDDATA/run

# Initialize $TASKDDATA.
log_line "Running 'taskd init --data \$TASKDDATA' --log"
OUTPUT=$($TASKD_BINARY init --data "$TASKDDATA" --log 2>&1)
if [ $? -eq 0 ]; then
  log_ok "Ok"
else
  log_error "Failed"
  log
  log "$OUTPUT"
  exit 1
fi

# Capture settings.
configure "Configuring host:port"     server        $HOST:$PORT
configure "Configuring log"           log           "$TASKDDATA/log/taskd.log"
configure "Configuring pid.file"      pid.file      "$TASKDDATA/run/taskd.pid"
configure "Configuring ip.log"        ip.log        1
configure "Configuring request.limit" request.limit 10485760
configure "Configuring ciphers"       ciphers       NORMAL

# Verify there are no certs in $DEFAULT_PKI
cd $DEFAULT_PKI
log_line "Preparing PEM files in $DEFAULT_PKI"
OUTPUT=$(ls *.pem 2>&1)
if [ $? -eq 0 ]; then
  log_warning "Existing PEM files found - they will be overwritten"
  log_line "Overwrite? [y|N]"
  read PROCEED
  PROCEED="${PROCEED:-N}"
  if [ "$PROCEED" != "y" ]; then
    log_error "Terminating"
    exit 1
  fi
else
  log_ok "Starting..."
fi

# TODO Verify there are no certs in $TASKDDATA/cert

log_line "Generating server CA cert"

OUTPUT=$(./generate.ca 2>&1)
if [ $? -eq 0 ]; then
  log_ok "Ok"

  log_line "Installing ca.cert.pem"
  cp $DEFAULT_PKI/ca.cert.pem $TASKDDATA/cert/ca.cert.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  configure "Configuring ca.cert" ca.cert $TASKDDATA/cert/ca.cert.pem
else
  log_error "Failed"
  log
  log "$OUTPUT"
  exit 1
fi

log_line "Generating server key/cert pair"
OUTPUT=$($DEFAULT_PKI/generate.server 2>&1)
if [ $? -eq 0 ]; then
  log_ok "Ok"

  log_line "Installing server.cert.pem"
  cp $DEFAULT_PKI/server.cert.pem $TASKDDATA/cert/server.cert.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  log_line "Installing server.key.pem"
  cp $DEFAULT_PKI/server.key.pem $TASKDDATA/cert/server.key.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  configure "Configuring server.cert" server.cert $TASKDDATA/cert/server.cert.pem
  configure "Configuring server.key"  server.key  $TASKDDATA/cert/server.key.pem
else
  log_error "Failed"
  log
  log "$OUTPUT"
  exit 1
fi

log_line "Generating server CRL"
OUTPUT=$($DEFAULT_PKI/generate.crl 2>&1)
if [ $? -eq 0 ]; then
  log_ok "Ok"

  log_line "Installing server.crl.pem"
  cp $DEFAULT_PKI/server.crl.pem $TASKDDATA/cert/server.crl.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  configure "Configuring server.crl" server.crl $TASKDDATA/cert/server.crl.pem
else
  log_error "Failed"
  log
  log "$OUTPUT"
  exit 1
fi

log_line "Generating API key/cert pair"
OUTPUT=$($DEFAULT_PKI/generate.client api 2>&1)
if [ $? -eq 0 ]; then
  log_ok "Ok"

  log_line "Installing api.cert.pem"
  cp $DEFAULT_PKI/api.cert.pem $TASKDDATA/cert/api.cert.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  log_line "Installing api.key.pem"
  cp $DEFAULT_PKI/api.key.pem $TASKDDATA/cert/api.key.pem
  if [ $? -eq 0 ]; then
    log_ok "Ok"
  else
    log_error "Failed"
    exit 1
  fi

  configure "Configuring api.cert" api.cert $TASKDDATA/cert/api.cert.pem
  configure "Configuring api.key"  api.key  $TASKDDATA/cert/api.key.pem
else
  log_error "Failed"
  log
  log "$OUTPUT"
  exit 1
fi

# Setup a number of users.
log_line "Would you like to set up a user account now? [y|N]"
read ANSWER
ANSWER="${ANSWER:-N}"
if [ "$ANSWER" == "y" ]; then
  log_line
  log_ok "Setting up user account"

  while [ 1 ]; do
    log_line "Enter the organization name for the user account [PUBLIC]"
    read ORGNAME
    ORGNAME="${ORGNAME:-PUBLIC}"

    if [ -n "$ORGNAME" ]; then

      log_line "Enter the full user name (e.g. First Last)"
      read USERNAME
      if [ -n "$USERNAME" ]; then

        # Create org if necessary
        log_line "Creating organization '$ORGNAME'"
        if [ -d "$TASKDDATA/orgs/$ORGNAME" ]; then
          log_ok "Already created"
        else
          OUTPUT=$($TASKD_BINARY add --data $TASKDDATA org "$ORGNAME" 2>&1)
          if [ $? -eq 0 ]; then
            log_ok "Ok"
          else
            log_error "Failed"
            log
            log "$OUTPUT"
            exit 1
          fi
        fi

        # Create user
        log_line "Creating user '$USERNAME'"
        OUTPUT=$($TASKD_BINARY add --data $TASKDDATA user --quiet "$ORGNAME" "$USERNAME" 2>&1)
        if [ $? -eq 0 ]; then
          log_ok "Ok"
          USERKEY=${OUTPUT:14}

          # Create user key/cert
          USERFILE=${USERNAME// /_}

          log_line "Generating $USERNAME key/cert pair"
          OUTPUT=$($DEFAULT_PKI/generate.client $USERFILE 2>&1)
          if [ $? -eq 0 ]; then
            log_ok "Ok"
          else
            log_error "Failed"
            log
            log "$OUTPUT"
            exit 1
          fi

          # Show tw configuration details
          log
          log "Taskwarrior Configuration details for $USERNAME."
          log
          log "  $USERNAME must copy these files to ~/.task"
          log "    $DEFAULT_PKI/ca.cert.pem"
          log "    $DEFAULT_PKI/$USERFILE.cert.pem"
          log "    $DEFAULT_PKI/$USERFILE.key.pem"
          log
          log "  $USERNAME must run these commands:"
          log "    task config taskd.server       $HOST:$PORT"
          log "    task config taskd.credentials  '$ORGNAME/$USERNAME/$USERKEY'"
          log "    task config taskd.certificate  ~/.task/$USERFILE.cert.pem"
          log "    task config taskd.key          ~/.task/$USERFILE.key.pem"
          log "    task config taskd.ca           ~/.task/ca.cert.pem"
          log "    task config taskd.trust        strict"
          log "    task config taskd.ciphers      NORMAL"
          log
        else
          log_error "Failed"
          log
          log "$OUTPUT"
          exit 1
        fi
      else
        log_error "Missing data"
        log
        log "You must specify an account name."
        exit 1
      fi
    else
      log_error "Missing data"
      log
      log "You must specify an organization."
      exit 1
    fi

    log_line "Create another user account? [y|N]"
    read ANSWER
    ANSWER="${ANSWER:-N}"
    if [ "$ANSWER" != "y" ]; then
      break;
    fi
  done
fi

# TODO Verify port is not in use

# Launch the server.
log
log "Your Taskserver is configured and ready."
log_line "Would you like to launch the server daemon? [y|N]"
read ANSWER
ANSWER="${ANSWER:-N}"
if [ "$ANSWER" == "y" ]; then
  log_line
  log_ok "Launching"
  OUTPUT=$(TASKDDATA=$TASKDDATA $TASKDCTL_BINARY start 2>&1)
  if [ $? -eq 0 ]; then
    log_line
    log_ok "Launched"
  else
    log_error "Failed"
    log
    log "$OUTPUT"
    exit 1
  fi

  # TODO Verify server running via log file
fi

log "Launch your Taskserver using this command:"
log
log "    TASKDDATA=\"$TASKDDATA\" \"$TASKDCTL_BINARY\" start"
log
log "Shut down your Taskserver using this command:"
log
log "    TASKDDATA=\"$TASKDDATA\" \"$TASKDCTL_BINARY\" stop"
log

log_ok "Done"
exit 0

################################################################################
