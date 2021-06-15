cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0079 NEW)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "None")
  message(STATUS "CMAKE_BUILD_TYPE is 'None', defaulting to 'Release'")
  set(CMAKE_BUILD_TYPE "Release")
endif()

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

install(TARGETS lsoracle CONFIGURATIONS RELEASE RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES core/test.ini CONFIGURATIONS RELEASE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/lsoracle)

enable_testing()
add_executable(unit_tests tests/basic.cpp)
target_link_libraries(unit_tests gtest_main)
include(GoogleTest)
gtest_discover_tests(unit_tests)
