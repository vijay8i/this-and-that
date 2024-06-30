#!/usr/bin/env python3

import sys
from utils import run


def main():
    cmd = ["conan"] + sys.argv[1:]
    run(cmd, check=True)


if __name__ == "__main__":
    main()
