#!/usr/bin/env python3
# Copyright 2018 The ChromiumOS Authors
# Copyright 2024 Million Views, LLC
#
import shlex
import subprocess
import sys
import os
import errno

"""Collection of utility functions used by the build system"""


def get_shell_output(cmd, args=[]):
    # print(f"cmd -> {cmd, args}")
    """Run |cmd + args| and return output as a list.

    Note: cmd is a list (not string); the interface allows you to
    pass the args as part of the cmd itself or split command to
    be the name of the command to be executed and pass the args in 
    the 'args' parameter.
    """
    result = subprocess.run(
        cmd + args, encoding="utf-8", stdout=subprocess.PIPE, check=False
    )
    if result.returncode:
        sys.exit(result.returncode)
    return shlex.split(result.stdout)


def find_project_root():
    """Find the project root by locating .gn file

    Navigates up from current working directory to find the directory
    containing '.gn' file till the root is reached.
    """
    current_path = os.getcwd()

    # Navigate up the directory tree to find the directory containing the '.gn' file
    # Stop when reaching the root
    while current_path != os.path.dirname(current_path):
        if '.gn' in os.listdir(current_path):
            return current_path
        current_path = os.path.dirname(current_path)
    return None


def set_config_path(path):
    if (path is None):
        path = find_project_root()

    if 'PKG_CONFIG_PATH' not in os.environ:
        os.environ['PKG_CONFIG_PATH'] = path
    else:
        os.environ['PKG_CONFIG_PATH'] += ':' + path
