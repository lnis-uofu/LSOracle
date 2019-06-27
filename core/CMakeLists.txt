cmake_minimum_required (VERSION 3.9)                  # CMake minimum version
project(lstools)

include_directories(${PROJECT_SOURCE_DIR}/lib/kahypar/include)  # add OpenTimer include
#include_directories(${PROJECT_SOURCE_DIR}/lib/darknet/include)

include_directories(${PROJECT_SOURCE_DIR}/../lib/oracle/include/oracle/classification/fplus/include)
include_directories(${PROJECT_SOURCE_DIR}/../lib/oracle/include/oracle/classification/eigen)
include_directories(${PROJECT_SOURCE_DIR}/../lib/oracle/include/oracle/classification/json/include)
include_directories(${PROJECT_SOURCE_DIR}/../lib/oracle/include/oracle/classification/fdeep_keras/include)

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)                        # thread library (pthread)
#find_package(OPENMP REQUIRED)

add_executable(lstools lstools.cpp)
#target_compile_options(lstools PRIVATE -fopenmp -v)
#target_link_libraries(lstools PUBLIC OpenMP::OpenMP_CXX oracle mockturtle kitty alice libabc OpenTimer Threads::Threads stdc++fs kahypar)

target_include_directories(lstools PRIVATE ../lib/kahypar/include)
#target_include_directories(lstools PRIVATE ../lib/darknet/include)
#darklib removed
target_link_libraries(lstools OpenMP::OpenMP_CXX oracle mockturtle kitty alice libabc OpenTimer Threads::Threads stdc++fs kahypar)
