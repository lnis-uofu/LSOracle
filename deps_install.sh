#!/bin/sh
if [! -d $DEPS_DIR/abc]; then
    cd $DEPS_DIR
    git clone https://github.com/berkeley-abc/abc.git
    cd abc && make ABC_USE_NO_READLINE=1
else 
    cd $DEPS_DIR/abc
    git pull https://github.com/berkeley-abc/abc.git
    make ABC_USE_NO_READLINE=1
fi