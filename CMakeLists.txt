cmake_minimum_required(VERSION 3.9)
project(lsoracle_app LANGUAGES CXX)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(core)
