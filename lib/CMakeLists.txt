#set(ABC_USE_NAMESPACE "abc")
#add_subdirectory(abc EXCLUDE_FROM_ALL)
#target_compile_definitions(libabc PUBLIC "LIN64")

set(BUILD_GTEST ON)
set(BUILD_GMOCK ON)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
target_compile_options(gmock PRIVATE -w)
target_compile_options(gtest PRIVATE -w)
include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(SYSTEM ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

add_subdirectory(pybind11)

add_subdirectory(kahypar EXCLUDE_FROM_ALL)
target_compile_definitions(kahypar PUBLIC "LIN64")
set_target_properties(kahypar PROPERTIES PUBLIC_HEADER "lib/kahypar/include/libkahypar.h")
target_link_libraries(kahypar INTERFACE pybind11 mini_boost gtest gmock)
target_compile_options(kahypar PRIVATE -w)
target_compile_options(mini_boost PRIVATE -w)
message(STATUS ${gtest_SOURCE_DIR})
target_include_directories(kahypar INTERFACE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_include_directories(kahypar INTERFACE ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

if (${ENABLE_GALOIS})
  add_subdirectory(Galois EXCLUDE_FROM_ALL)
endif()

add_subdirectory(alice)
target_include_directories(alice INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/alice)
target_compile_definitions(alice INTERFACE "-DREADLINE_USE_READLINE=1")
target_link_libraries(alice INTERFACE any cli11 fmt json readline pybind11)

add_library(any INTERFACE)
target_include_directories(any INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/any)

add_library(cli11 INTERFACE) # v1.3
target_include_directories(cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)

add_library(fmt INTERFACE) # v4.1.0
target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY)

add_library(json INTERFACE) # v3.0.0
target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Threads REQUIRED)                        # thread library (pthread)

add_subdirectory(mockturtle)
target_compile_options(mockturtle INTERFACE -w)

#set(PERCY_BUILD_KITTY OFF CACHE BOOL "Build kitty for percy" FORCE)
#add_subdirectory(percy)
