cmake_minimum_required(VERSION 3.9)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

project(alice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENSTA_HOME
  ${PROJECT_SOURCE_DIR}/module/OpenSTA
)

# some specific compiler definitions
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fcolor-diagnostics" HAS_FCOLOR_DIAGNOSTICS)
if (HAS_FCOLOR_DIAGNOSTICS)
  add_compile_options(-fcolor-diagnostics)
endif()

add_subdirectory(${OPENSTA_HOME})
add_subdirectory(lib)
#add_subdirectory(examples)
add_subdirectory(OpenTimer)
include_directories(${PROJECT_SOURCE_DIR}/OpenTimer)  # add OpenTimer include

set(CMAKE_CXX_STANDARD 17)                            # enable c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#define TK paths for 
set(TK_LIBRARY
/usr/lib64/libtk8.5.so
)

set(TK_INCLUDE_PATH
/include/gtk-3.0/gtk/gtk.h
)

find_package(TCL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SWIG REQUIRED)

find_package(Threads REQUIRED)                        # thread library (pthread)
add_subdirectory(core)
