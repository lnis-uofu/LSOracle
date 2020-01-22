cmake_minimum_required (VERSION 3.9)                  # CMake minimum version
project(lsoracle)

include_directories(${PROJECT_SOURCE_DIR}/lib/kahypar/include)
#include_directories(${PROJECT_SOURCE_DIR}/lib/Galois/lonestar) 

include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/fplus/include)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/eigen)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/json/include)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/fdeep_keras/include)

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lsoracle lsoracle.cpp)

target_include_directories(lsoracle PRIVATE ../lib/kahypar/include)
#target_include_directories(lsoracle PRIVATE ../lib/Galois/lonestar)
#target_link_libraries(lsoracle alice mockturtle stdc++fs kahypar galois_utah)
target_link_libraries(lsoracle alice mockturtle stdc++fs kahypar)
