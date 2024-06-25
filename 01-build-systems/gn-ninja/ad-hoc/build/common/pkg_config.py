#!/usr/bin/env python3
# Copyright 2018 The ChromiumOS Authors
# Copyright 2024 Million Views, LLC
#
import json
import sys
import errno
import argparse
from utils import get_shell_output, set_config_path

"""
Parses pkg-config output and formats it into json for use in GN

Usage:
  pkg-config.py [-path PATH] pkg1 pkg2 ...
"""


def main(path, packages):
    set_config_path(path)
    # cmd = ["pkg-config", "--silence-errors"]
    cmd = ["pkg-config"]
    cflags = []
    include_dirs = []
    for cflag in get_shell_output(cmd, packages + ["--cflags"]):
        if cflag.startswith("-D"):
            # strip -D
            cflags.append(cflag)
        elif cflag.startswith("-I"):
            # strip -I
            include_dirs.append(cflag[2:])
        else:
            sys.exit(errno.ENOTSUP)

    libs = []
    lib_dirs = []
    ldflags = []
    for ldflag in get_shell_output(cmd, packages + ["--libs"]):
        if ldflag.startswith("-l"):
            # strip -l.
            libs.append(ldflag[2:])
        elif ldflag.startswith("-L"):
            # strip -L.
            lib_dirs.append(ldflag[2:])
        else:
            ldflags.append(ldflag)
    # Set sort_keys=True for stabilization.
    result = {
        "cflags": cflags,
        "include_dirs": include_dirs,
        "libs": libs,
        "lib_dirs": lib_dirs,
        "ldflags": ldflags,
    }
    json.dump(result, sys.stdout, sort_keys=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Return metainfo of installed libraries (relative to a path, optionally).")
    parser.add_argument('-p', '--path', type=str,
                        help='Optional path argument')
    parser.add_argument('package-name', nargs='+',
                        help='List of package names')

    args = parser.parse_args()
    sys.exit(main(args.path, getattr(args, "package-name")))
