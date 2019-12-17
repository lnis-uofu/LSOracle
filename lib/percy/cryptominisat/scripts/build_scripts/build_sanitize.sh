#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include tests cusp* scalmc*
CXX=clang++ cmake -DENABLE_PYTHON_INTERFACE=ON -DENABLE_TESTING=ON -DSANITIZE=ON ..
make -j6
make test
