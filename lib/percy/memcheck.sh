#!/bin/bash

for filename in ./build/test/*; do
    if [ "$filename" == "./build/test/Makefile" ]; then
        continue
    fi
    if [ "$filename" == "./build/test/CMakeFiles" ]; then
        continue
    fi
    if [ "$filename" == "./build/test/cmake_install.cmake" ]; then
        continue
    fi
    if [ "$filename" == "./build/test/CTestTestfile.cmake" ]; then
        continue
    fi
    echo "Checking memory of $filename"
    if ! valgrind --leak-check=full --error-exitcode=1 $filename; then
        echo "Memory check failed!"
        exit 1
    fi
done
