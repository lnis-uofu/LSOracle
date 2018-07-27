#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include scalmc* cusp*
rm -rf tests
cmake -DUSE_GAUSS=ON -DENABLE_TESTING=ON ..
make -j6
make test
