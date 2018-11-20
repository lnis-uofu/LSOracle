#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include scalmc* cusp*
rm -rf tests
cmake -DSTATICCOMPILE=ON -DUSE_GAUSS=ON ..
make -j6
