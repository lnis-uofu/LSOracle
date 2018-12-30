#!/usr/bin/env python
from __future__ import with_statement
import sys
import string

print("c %s" % sys.argv)


# create header
headerNumVars = 0
headerNumCls = 0
at = 0
for fname in sys.argv:
    at += 1
    if at == 1:
        continue

    thisnumvars = 0
    with open(fname, "r") as ins:
        for line in ins:
            if line[0] == 'p' or line[0] == 'c':
                continue

            for part in line.split():
                if part.strip() == 'x':
                    continue
                thisnumvars = max(thisnumvars, abs(int(part)))

            headerNumCls += 1
        headerNumVars += thisnumvars

print("p cnf %d %d" % (headerNumVars, headerNumCls))

# print final CNF
ret = ""
at = 0
numvarsUntilNow = 0
for f in sys.argv:
    at += 1
    if at == 1:
        continue

    thisnumvars = 0
    with open(f, "r") as ins:
        for line in ins:
            # ignore header and comments
            if line[0] == 'p' or line[0] == 'c':
                continue

            line = line.rstrip().lstrip()
            parts = line.split()
            towrite = ""
            for part in parts:
                if part == "x":
                    towrite += "x "
                    continue

                # end of line
                if (part == "0"):
                    towrite += "0"
                    break

                # update number of variables in this part
                thisnumvars = max(thisnumvars, abs(int(part)))

                # increment variable number if need be
                newLit = abs(int(part)) + numvarsUntilNow

                # invert if needed
                if (int(part) < 0):
                    newLit = -1*newLit

                # write updated literal
                towrite += "%d " % newLit

            # end of this line in file
            print(towrite)

    # next part has to be updated with incremented varaibles
    numvarsUntilNow += thisnumvars
