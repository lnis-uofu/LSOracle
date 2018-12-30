#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2018  Mate Soos
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


import sys
f = open(sys.argv[1], "r")
maxvar = 0
for line in f :
    if len(line) == 0:
        continue

    for v in line.split() :
        v = v.lstrip().rstrip()
        if v == "d" :
            continue

        maxvar = max(maxvar, abs(int(v)))

print "maxvar: ", maxvar
