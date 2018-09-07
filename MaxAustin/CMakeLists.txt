cmake_minimum_required(VERSION 3.8)
project(alice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# some specific compiler definitions
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fcolor-diagnostics" HAS_FCOLOR_DIAGNOSTICS)
if (HAS_FCOLOR_DIAGNOSTICS)
  add_compile_options(-fcolor-diagnostics)
endif()

add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(testing)
