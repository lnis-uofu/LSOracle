#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include
CXX=clang++ cmake -DENABLE_TESTING=ON -DSLOW_DEBUG=ON -DNOVALGRIND=OFF ..
make -j6 VERBOSE=1
make test
