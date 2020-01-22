#set(ABC_USE_NAMESPACE "abc")
#add_subdirectory(abc EXCLUDE_FROM_ALL)
#target_compile_definitions(libabc PUBLIC "LIN64")

add_subdirectory(kahypar EXCLUDE_FROM_ALL)
target_compile_definitions(kahypar PUBLIC "LIN64")

#add_subdirectory(Galois EXCLUDE_FROM_ALL)

add_subdirectory(alice)
target_include_directories(alice INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/alice)
target_compile_definitions(alice INTERFACE "-DREADLINE_USE_READLINE=1")
target_link_libraries(alice INTERFACE any cli11 fmt json readline)

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

#set(PERCY_BUILD_KITTY OFF CACHE BOOL "Build kitty for percy" FORCE)
#add_subdirectory(percy)
