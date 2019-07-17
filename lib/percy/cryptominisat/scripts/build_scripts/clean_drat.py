#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import sys
import unittest


def fix_line(line):
    toprint = ""
    for e in line.split():
        e = e.strip()
        if e == "d":
            toprint += "d "
            continue

        # print("e is %s" % e)
        if int(e) == 0:
            toprint += "0"
            break

        toprint += "%d " % int(e)

    return toprint


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Error: must give file to parse")
        exit(-1)

    fname = sys.argv[1]

    f = open(fname)

    for line in f:
        print(fix_line(line))

    f.close()
