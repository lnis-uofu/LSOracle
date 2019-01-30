cmake_minimum_required (VERSION 3.9)                  # CMake minimum version
project(lstools)

#include_directories(${PROJECT_SOURCE_DIR}/OpenTimer/)  # add OpenTimer include

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)                        # thread library (pthread)

add_executable(lstools lstools.cpp)
#target_link_libraries(lstools mockturtle kitty alice libabc)
target_link_libraries(lstools mockturtle kitty alice libabc OpenTimer Threads::Threads stdc++fs KaHyPar)
#target_link_libraries(lstools ../lib/OpenTimer Threads::Threads stdc++fs mockturtle kitty alice libabc )
