#!/bin/sh
DEPS_DIR="${HOME}/deps"
mkdir -p ${DEPS_DIR} && cd ${DEPS_DIR}
CMAKE_URL="https://cmake.org/files/v3.13/cmake-3.13.0-rc3-Linux-x86_64.tar.gz"
mkdir -p cmake && travis_retry wget --no-check-certificate --quiet -O - ${CMAKE_URL} | tar --strip-components=1 -xz -C cmake
export PATH=${DEPS_DIR}/cmake/bin:${PATH}

if [! -d "$DEPS_DIR/abc"]; then
    git clone https://github.com/berkeley-abc/abc.git
    cd abc && make ABC_USE_NO_READLINE=1
else 
    cd abc
    git pull https://github.com/berkeley-abc/abc.git
    make ABC_USE_NO_READLINE=1
fi