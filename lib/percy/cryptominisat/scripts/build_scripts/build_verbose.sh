#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include
rm -rf tests
cmake -DENABLE_TESTING=ON ..
make -j6 VERBOSE=1
ctest -V
