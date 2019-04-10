#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include tests
CC=afl-clang-fast CXX=afl-clang-fast++ cmake -DLIMITMEM=ON -DENABLE_PYTHON_INTERFACE=OFF -DENABLE_TESTING=OFF -DNOZLIB=ON ..
make -j6
afl-fuzz -M master -i -  -o afl/findings_dir/ ./cryptominisat5 --debuglib 1
