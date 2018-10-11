# -*- coding: utf-8 -*-

import atexit
import json
import os
import shlex
import shutil
import tempfile
import unittest
from .exceptions import CommandError
from .hooks import Hooks
from .utils import run_cmd_wait, run_cmd_wait_nofail, which, taskd_binary_location
from .compat import STRING_TYPE


class Taskd(object):
    """Manage a taskserver instance

    A temporary folder is used as data store of taskserver

    A taskd client should not be used after being destroyed.
    """
    DEFAULT_TASKD = taskd_binary_location()

    def __init__(self, taskd=DEFAULT_TASKD):
        """Initialize a Taskserver. The taskd client runs in a temporary folder.

        """
        self.taskd = taskd

        # Used to specify what command to launch (and to inject faketime)
        self._command = [self.taskd]

        # Configuration of the isolated environment
        self._original_pwd = os.getcwd()
        self.datadir = tempfile.mkdtemp(prefix="taskd_")
        os.environ["TASKDDATA"] = self.datadir

        # Ensure any instance is properly destroyed at session end
        atexit.register(lambda: self.destroy())

        self.reset_env()

    def __repr__(self):
        txt = super(Taskd, self).__repr__()
        return "{0} running from {1}>".format(txt[:-1], self.datadir)

    def __call__(self, *args, **kwargs):
        "aka t = Taskd() ; t() which is now an alias to t.runSuccess()"
        return self.runSuccess(*args, **kwargs)

    def reset_env(self):
        """Set a new environment derived from the one used to launch the test
        """
        # Copy all env variables to avoid clashing subprocess environments
        self.env = os.environ.copy()

        # Make sure TASKDDATA is isolated
        self.env["TASKDDATA"] = self.datadir

    def config(self, var, value):
        """Run setup `var` as `value` in taskd config
        """
        # Add -- to avoid misinterpretation of - in things like UUIDs
        cmd = (self.taskd, "config", "--force", var, value)
        return run_cmd_wait(cmd, env=self.env)

    def del_config(self, var):
        """Remove `var` from taskd config
        """
        cmd = (self.taskd, "config", "--force", var)
        return run_cmd_wait(cmd, env=self.env)

    @staticmethod
    def _split_string_args_if_string(args):
        """Helper function to parse and split into arguments a single string
        argument. The string is literally the same as if written in the shell.
        """
        # Enable nicer-looking calls by allowing plain strings
        if isinstance(args, STRING_TYPE):
            args = shlex.split(args)

        return args

    def runSuccess(self, args="", input=None, merge_streams=False,
                   timeout=5):
        """Invoke taskd with given arguments and fail if exit code != 0

        Use runError if you want exit_code to be tested automatically and
        *not* fail if program finishes abnormally.

        If you wish to pass instructions to taskd such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny\n".

        If merge_streams=True stdout and stderr will be merged into stdout.

        timeout = number of seconds the test will wait for every taskd call.
        Defaults to 1 second if not specified. Unit is seconds.

        Returns (exit_code, stdout, stderr) if merge_streams=False
                (exit_code, output) if merge_streams=True
        """
        # Create a copy of the command
        command = self._command[:]

        args = self._split_string_args_if_string(args)
        command.extend(args)

        output = run_cmd_wait_nofail(command, input,
                                     merge_streams=merge_streams,
                                     env=self.env,
                                     timeout=timeout)

        if output[0] != 0:
            raise CommandError(command, *output)

        return output

    def runError(self, args=(), input=None, merge_streams=False, timeout=5):
        """Invoke taskd with given arguments and fail if exit code == 0

        Use runSuccess if you want exit_code to be tested automatically and
        *fail* if program finishes abnormally.

        If you wish to pass instructions to taskd such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny\n".

        If merge_streams=True stdout and stderr will be merged into stdout.

        timeout = number of seconds the test will wait for every taskd call.
        Defaults to 1 second if not specified. Unit is seconds.

        Returns (exit_code, stdout, stderr) if merge_streams=False
                (exit_code, output) if merge_streams=True
        """
        # Create a copy of the command
        command = self._command[:]

        args = self._split_string_args_if_string(args)
        command.extend(args)

        output = run_cmd_wait_nofail(command, input,
                                     merge_streams=merge_streams,
                                     env=self.env,
                                     timeout=timeout)

        # output[0] is the exit code
        if output[0] == 0 or output[0] is None:
            raise CommandError(command, *output)

        return output

    def destroy(self):
        """Cleanup the data folder and release server port for other instances
        """
        try:
            shutil.rmtree(self.datadir)
        except OSError as e:
            if e.errno == 2:
                # Directory no longer exists
                pass
            else:
                raise

        # Prevent future reuse of this instance
        self.runSuccess = self.__destroyed
        self.runError = self.__destroyed

        # self.destroy will get called when the python session closes.
        # If self.destroy was already called, turn the action into a noop
        self.destroy = lambda: None

    def __destroyed(self, *args, **kwargs):
        raise AttributeError("Taskd instance has been destroyed. "
                             "Create a new instance if you need a new client.")

    @classmethod
    def not_available(cls):
        """Check if the taskd binary is available in the path"""
        if which(cls.DEFAULT_TASKD):
            return False
        else:
            return True

    def diag(self, merge_streams_with=None):
        """Run taskd diagnostics.

        This function may fail in which case the exception text is returned as
        stderr or appended to stderr if merge_streams_with is set.

        If set, merge_streams_with should have the format:
        (exitcode, out, err)
        which should be the output of any previous process that failed.
        """
        try:
            output = self.runSuccess("diag")
        except CommandError as e:
            # If taskd diag failed add the error to stderr
            output = (e.code, None, str(e))

        if merge_streams_with is None:
            return output
        else:
            # Merge any given stdout and stderr with that of "taskd diag"
            code, out, err = merge_streams_with
            dcode, dout, derr = output

            # Merge stdout
            newout = "\n##### Debugging information (taskd diag): #####\n{0}"
            if dout is None:
                newout = newout.format("Not available, check STDERR")
            else:
                newout = newout.format(dout)

            if out is not None:
                newout = out + newout

            # And merge stderr
            newerr = "\n##### Debugging information (taskd diag): #####\n{0}"
            if derr is None:
                newerr = newerr.format("Not available, check STDOUT")
            else:
                newerr = newerr.format(derr)

            if err is not None:
                newerr = err + derr

            return code, newout, newerr

    def faketime(self, faketime=None):
        """Set a faketime using libfaketime that will affect the following
        command calls.

        If faketime is None, faketime settings will be disabled.
        """
        cmd = which("faketime")
        if cmd is None:
            raise unittest.SkipTest("libfaketime/faketime is not installed")

        if self._command[0] == cmd:
            self._command = self._command[3:]

        if faketime is not None:
            # Use advanced time format
            self._command = [cmd, "-f", faketime] + self._command

# vim: ai sts=4 et sw=4
