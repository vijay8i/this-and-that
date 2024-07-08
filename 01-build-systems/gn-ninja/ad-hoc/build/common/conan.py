#!/usr/bin/env python3

import sys
from utils import run, find_project_root, log_process_info


def main():
    cmd = ["conan"] + sys.argv[1:]
    run(cmd, check=True)


if __name__ == "__main__":
    log_process_info(find_project_root() + "/" + "conan-install.log")
    main()
