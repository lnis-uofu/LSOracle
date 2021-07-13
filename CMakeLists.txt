cmake_minimum_required(VERSION 3.9)
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
option(ENABLE_GALOIS
  "Enable Galois library for hypergraph partitioning." OFF)

option(ENABLE_LIBABC
  "Enable libabc library." OFF)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

project(alice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# some specific compiler definitions
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fcolor-diagnostics" HAS_FCOLOR_DIAGNOSTICS)
if (HAS_FCOLOR_DIAGNOSTICS)
  add_compile_options(-fcolor-diagnostics)
endif()

add_subdirectory(lib)

find_package(Threads REQUIRED)                        # thread library (pthread)
add_subdirectory(core)

# Link against LibRT on older Linux distros
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_library(LIBRT rt)
  target_link_libraries(lsoracle ${LIBRT})
endif()

install(TARGETS lsoracle CONFIGURATIONS RELEASE RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES core/test.ini CONFIGURATIONS RELEASE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/lsoracle)

if (${YOSYS_PLUGIN})
  if (NOT DEFINED YOSYS_INCLUDE_DIR)
    message(FATAL_ERROR "YOSYS_INCLUDE_DIR was not set, but plugin build was requested. If using a system installed yosys, this is typically located in /usr/local/share/yosys/include. In the yosys repository, the directory is ./share/include, or can also be specified as the root of the repository.")
  endif()
  add_subdirectory(yosys-plugin)
  if (DEFINED YOSYS_SHARE_DIR)
    install(TARGETS yosys-plugin CONFIGURATIONS RELEASE DESTINATION ${YOSYS_SHARE_DIR}/plugin)
  endif()
endif()

enable_testing()
add_executable(unit_tests tests/basic.cpp)
target_link_libraries(unit_tests gtest_main)
include(GoogleTest)
gtest_discover_tests(unit_tests)
