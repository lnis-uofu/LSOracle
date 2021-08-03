cmake_minimum_required(VERSION 3.9)
project(lsoracle_app LANGUAGES CXX)
cmake_policy(SET CMP0079 NEW)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE STREQUAL "None")
  # "None" is specifically checked, because launchpad builders set the value to None when invoking cmake
  message(STATUS "Setting build type to 'Release' as no value or 'None' was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Build type is '${CMAKE_BUILD_TYPE}'")

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  message(WARNING "Build type is DEBUG, be aware that performance will be unusable for any non-trivial circuit.")
endif()

option(YOSYS_PLUGIN "compile the yosys plugin" OFF)
option(LOCAL_PYBIND "use pybind11 from submodule" ON)
option(LOCAL_BOOST "use boost from submodules" ON)
option(LOCAL_GTEST "use googletest from submodules" ON)
option(ENABLE_GALOIS
  "Enable Galois library for hypergraph partitioning." OFF)

option(ENABLE_LIBABC
  "Enable libabc library." OFF)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(core)

if (${YOSYS_PLUGIN})
  add_subdirectory(yosys-plugin)
endif()
