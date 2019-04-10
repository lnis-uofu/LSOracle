#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include tests
cmake -DENABLE_PYTHON_INTERFACE=ON ..
make -j6
