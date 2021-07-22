cmake_minimum_required (VERSION 3.9)                  # CMake minimum version
project(lsoracle)

include_directories(../lib/kahypar/include)
include_directories(.)

include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/fplus/include)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/eigen)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/json/include)
include_directories(${PROJECT_SOURCE_DIR}/algorithms/classification/fdeep_keras/include)

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  COMMAND python3 ${PROJECT_SOURCE_DIR}/generate_kahypar.py ${PROJECT_SOURCE_DIR}/test.ini ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/test.ini
)

add_executable(lsoracle lsoracle.cpp ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp kahypar_temp_config.cpp)
target_include_directories(lsoracle PRIVATE ../lib/kahypar/include)
target_include_directories(lsoracle PRIVATE .)

add_library(liblsoracle lsoracle.cpp)
target_include_directories(liblsoracle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(liblsoracle INTERFACE ../lib/kahypar/include)
set_target_properties(liblsoracle PROPERTIES PREFIX "")

target_link_libraries(lsoracle alice mockturtle kahypar)
target_link_libraries(liblsoracle alice mockturtle kahypar)

if (${ENABLE_GALOIS})
  add_definitions(-DENABLE_GALOIS)
  target_include_directories(lsoracle PRIVATE ../lib/Galois/lonestar)
  target_link_libraries(lsoracle galois_utah)
endif()


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0 OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
  target_link_libraries(lsoracle stdc++fs)
  target_link_libraries(liblsoracle stdc++fs)
endif()

configure_file(test.ini test.ini COPYONLY)
