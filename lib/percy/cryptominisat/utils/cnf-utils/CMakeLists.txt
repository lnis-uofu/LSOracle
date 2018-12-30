# Copyright (c) 2017, Mate Soos
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(POLICY CMP0048)
    #policy for VERSION in cmake 3.0
    cmake_policy(SET CMP0048 OLD)
endif()

IF(DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

PROJECT(cnf-utils)

#default flags
if (NOT MSVC)
    add_compile_options( -g)

    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:-O3>")
    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:-mtune=native>")

    add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
    add_compile_options("$<$<CONFIG:RELEASE>:-g0>")
    add_compile_options("$<$<CONFIG:RELEASE>:-DNDEBUG>")
    add_compile_options("$<$<CONFIG:RELEASE>:-mtune=native>")

    add_compile_options("$<$<CONFIG:DEBUG>:-O0>")

else()
    # see https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx for details
    # /ZI = include debug info
    # /Wall = all warnings

    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:/Ox>")
    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:/ZI>")

    add_compile_options("$<$<CONFIG:RELEASE>:/Ox>")
    add_compile_options("$<$<CONFIG:RELEASE>:/D>")
    add_compile_options("$<$<CONFIG:RELEASE>:/NDEBUG>")
    add_compile_options("$<$<CONFIG:RELEASE>:/ZI>")

    add_compile_options("$<$<CONFIG:DEBUG>:/Od>")

    # We statically link to reduce dependencies
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MD")
        if(${flag_var} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MDd")
    endforeach(flag_var)

    # buffers security check
    add_compile_options(/GS)

    # Proper warning level
    add_compile_options(/W1)
    # Disable STL used in DLL-boundary warning
    add_compile_options(/wd4251)
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/fp:precise)

    # exception handling. s = The exception-handling model that catches C++ exceptions only and tells the compiler to assume that functions declared as extern "C" may throw an exception.
    # exception handling. c = If used with s (/EHsc), catches C++ exceptions only and tells the compiler to assume that functions declared as extern "C" never throw a C++ exception.
    add_compile_options(/EHsc)

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PDBCOMPRESS")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:1572864")

    #what does this do?
    set(DEF_INSTALL_CMAKE_DIR CMake)
endif()

include(CheckCXXCompilerFlag)
macro(add_cxx_flag_if_supported flagname)
  check_cxx_compiler_flag("${flagname}" HAVE_FLAG_${flagname})

  if(HAVE_FLAG_${flagname})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flagname}" )
  endif()
endmacro()

if (NOT WIN32)
    add_cxx_flag_if_supported("-Wall")
    add_cxx_flag_if_supported("-Wextra")
    add_cxx_flag_if_supported("-Wunused")
    add_cxx_flag_if_supported("-pedantic")
    add_cxx_flag_if_supported("-Wsign-compare")
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_cxx_flag_if_supported("-fno-omit-frame-pointer")
    endif()
    add_cxx_flag_if_supported("-Wtype-limits")
    add_cxx_flag_if_supported("-Wuninitialized")
    add_cxx_flag_if_supported("-Wno-deprecated")
    add_cxx_flag_if_supported("-Wstrict-aliasing")
    add_cxx_flag_if_supported("-Wpointer-arith")
    add_cxx_flag_if_supported("-Wheader-guard")
    add_cxx_flag_if_supported("-Wpointer-arith")
    add_cxx_flag_if_supported("-Wformat-nonliteral")
    add_cxx_flag_if_supported("-Winit-self")
    add_cxx_flag_if_supported("-Wparentheses")
    add_cxx_flag_if_supported("-Wunreachable-code")
    add_cxx_flag_if_supported("-ggdb3")
    add_compile_options("-fPIC")
endif()


# add_executable(cnf-cat
#     cnf-cat.cc
# )
# add_executable(cnf-clause
#     cnf-clause.cc
# )
# add_executable(cnf-grep
#     cnf-grep.cc
# )
# add_executable(cnf-shuffle-variables
#     cnf-shuffle-variables.cc
# )
# add_executable(cnf-shuffle-literals
#     cnf-shuffle-literals.cc
# )
# add_executable(cnf-shuffle-clauses
#     cnf-shuffle-clauses.cc
# )
# add_executable(cnf-sort-literals
#     cnf-sort-literals.cc
# )
# add_executable(cnf-sort-clauses
#     cnf-sort-clauses.cc
# )
# add_executable(cnf-propagate
#     cnf-propagate.cc
# )
# add_executable(cnf-stat
#     cnf-stat.cc
# )
# add_executable(cnf-complete
#     cnf-complete.cc
# )
#
# add_executable(cnf-renumber
#     cnf-renumber.c
# )
# add_executable(cnf-fuzz-nossum
#     cnf-fuzz-nossum.cc
# )
#
# add_executable(cnf-fuzz-nossum2
#     cnf-fuzz-nossum2.cc
# )
add_executable(cnf-fuzz-biere
    cnf-fuzz-biere.cc
)

add_executable(sgen4
    sgen4.c
)

add_executable(largefuzzer
    largefuzzer.cpp
)

if (NOT MSVC)
    target_link_libraries(sgen4 m)
endif()
