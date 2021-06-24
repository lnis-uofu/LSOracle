cmake_minimum_required(VERSION 3.1)
project(alice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ALICE_EXAMPLES "Build examples" OFF)
option(ALICE_TEST "Build tests" OFF)

set(ALICE_READLINE_LIBRARY READLINE CACHE STRING "Readline library")
set_property(CACHE ALICE_READLINE_LIBRARY PROPERTY STRINGS NATIVE READLINE)

# some specific compiler definitions
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fcolor-diagnostics" HAS_FCOLOR_DIAGNOSTICS)
if (HAS_FCOLOR_DIAGNOSTICS)
  add_definitions(-fcolor-diagnostics)
endif()

if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools")
include(AliceTools)

add_subdirectory(lib)
add_subdirectory(include)

if(ALICE_EXAMPLES)
  add_subdirectory(examples)
endif()

if(ALICE_TEST)
  add_subdirectory(test)
endif()

