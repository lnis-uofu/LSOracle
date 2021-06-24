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

Integration into own CMake projects
-----------------------------------

If you are using git, create a subdirectory of Alice into your project
directory::

  git submodule add https://github.com/msoeken/alice

Otherwise, it also suffices to move the contents of the Alice repository into
a folder called ``alice`` inside your project directory.  Inside your project's
*CMakeLists.txt* file (which is in the same directory in which there is the
alice directory), add the following line::

  add_subdirectory(alice)

before adding any other source files of your project.  Then, link alice to your
project using::

  target_link_libraries(myproject alice)

Building examples
-----------------

In order to build the examples, you need to enable them. Run the following from
the base directory of alice::

  git submodule update --init --recursive
  mkdir build
  cd build
  cmake -DALICE_EXAMPLES=ON ..
  make

Building tests
--------------

In order to run the tests and the micro benchmarks, you need to enable tests in
CMake::

  git submodule update --init --recursive
  mkdir build
  cd build
  cmake -DALICE_TEST=ON ..
  make
  ./test/run_tests
