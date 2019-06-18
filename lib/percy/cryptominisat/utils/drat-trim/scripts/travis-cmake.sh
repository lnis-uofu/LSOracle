#!/bin/bash

# Copyright (C) 2017  Mate Soos
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

# This file wraps CMake invocation for TravisCI
# so we can set different configurations via environment variables.

set -e
set -x

SOURCE_DIR=$(pwd)
cd build
BUILD_DIR=$(pwd)

# Note eval is needed so COMMON_CMAKE_ARGS is expanded properly
case $DRAT_CONFIG in
    ZLIB)
        eval cmake "${SOURCE_DIR}"
    ;;

    NOZLIB)
        sudo apt-get remove zlib1g-dev
        eval cmake "${SOURCE_DIR}"
    ;;

    INTREE_BUILD)
        cd ..
        SOURCE_DIR=$(pwd)
        BUILD_DIR=$(pwd)
        eval cmake "${SOURCE_DIR}"
    ;;

    *)
        echo "\"${CMS_CONFIG}\" configuration not recognised"
        exit 1
    ;;
esac

make

