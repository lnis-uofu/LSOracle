Installation
============

Percy is a header-only library. As such, no build steps are required to start
using it. Simply add ${PERCY_ROOT}/include to your compiler's include path and
you're good to go. However, if you want to run the tests, you'll need to build
some binaries. 

The build instructions depend on your operating system. On Unix-like operating
systems you will need either g++ (at least version 4.9.0) or clang++ (at least
version 3.5.0). On Windows you can build using Visual Studio. (NOTE: This has
been tested only with Visual Studio 2017.) Once those requirements are met, run
the following commands to build and run the tests:

.. code-block:: bash

    git clone --recurse-submodules https://github.com/whaaswijk/percy.git
    cd percy
    mkdir build
    cd build
    cmake .. -DPERCY_TEST=ON # Only on Unix
    cmake -DPERCY_TEST=ON -G "Visual Studio 15 2017" .. # On Windows
    make # Only on Unix
    make test # Only on Unix

On Unix systems this will build and run the test suite. On Windows it will
generate a solution file which allows you to build and run the tests using
Visual Studio.
