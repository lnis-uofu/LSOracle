#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include
rm -rf tests
cmake -DENABLE_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j6
make test
