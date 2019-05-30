#!/bin/sh
if [! -d $HOME/deps/abc]; then
    cd $HOME/deps
    git clone https://github.com/berkeley-abc/abc.git
    cd abc && make ABC_USE_NO_READLINE=1
else 
    cd $DEPS_DIR/abc
    git pull https://github.com/berkeley-abc/abc.git
    make ABC_USE_NO_READLINE=1 ||:
fi