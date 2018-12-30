#!/usr/bin/env python
"""
This simple script runs a program and inverts
its exit code.

e.g. 
$ not true
$ not ls -l
"""
import sys
import subprocess

if __name__ == '__main__':
    if len(sys.argv) < 2:
       sys.stderr.write('Must pass at least 1 argument\n')
       sys.exit(255)

    cmdLine = sys.argv[1:]
    p = subprocess.Popen(cmdLine)
    result = p.wait()

    if result > 0:
        sys.exit(0)
    else:
        sys.exit(1)
