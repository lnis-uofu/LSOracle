Installation
============

alice is a header-only C++-14 library.  Just add the include directory
of alice to your include directories, and you can integrate alice into
your source files using

.. code-block:: c++

   #include <alice/alice.hpp>

Compile with readline
---------------------

Alice can use the *readline* library to enable command completition and history.
If one integrates alice through *CMake*, then *readline* is enabled by default.
Otherwise, make sure to define ``READLINE_USE_READLINE`` and link against
*readline*.

Building examples
-----------------

Examples are enabled by default.  In order to build them, run the
following from the base directory of alice::

  mkdir build
  cd build
  cmake ..
  make

Building tests
--------------

In order to run the tests and the micro benchmarks, you need to enable tests in
CMake::

  mkdir build
  cd build
  cmake -DALICE_TEST=ON ..
  make
  ./test/run_tests
