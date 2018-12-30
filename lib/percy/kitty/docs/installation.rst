Installation
============

kitty is a header-only C++-14 library.  Just add the include directory
of kitty to your include directories, and you can integrate kitty into
your source files using

.. code-block:: c++

   #include <kitty/kitty.hpp>

Building examples
-----------------

Examples are enabled by default.  In order to build them, run the
following from the base directory of kitty::

  mkdir build
  cd build
  cmake ..
  make

Building tests and micro benchmarks
-----------------------------------

In order to run the tests and the micro benchmarks, you need to init
the submodules and enable tests in CMake::

  git submodule update --init --recursive
  mkdir build
  cd build
  cmake -DKITTY_BENCH=ON -DKITTY_TEST=ON ..
  make
  ./test/run_tests
  ./bench/bench_operations       # or any other benchmark
