cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(minisat_only_elim_and_subsume)

if(POLICY CMP0048)
    #policy for VERSION in cmake 3.0
    cmake_policy(SET CMP0048 OLD)
endif()

#--------------------------------------------------------------------------------------------------
# Configurable options:

option(STATIC_BINARIES "Link binaries statically." ON)
option(USE_SORELEASE   "Use SORELEASE in shared library filename." ON)

#--------------------------------------------------------------------------------------------------
# Library version:

set(MINISATOEAS_SOMAJOR   2)
set(MINISATOEAS_SOMINOR   1)
set(MINISATOEAS_SORELEASE 0)

# Compute VERSION and SOVERSION:
if (USE_SORELEASE)
  set(MINISATOEAS_VERSION ${MINISATOEAS_SOMAJOR}.${MINISATOEAS_SOMINOR}.${MINISATOEAS_SORELEASE})
else()
  set(MINISATOEAS_VERSION ${MINISATOEAS_SOMAJOR}.${MINISATOEAS_SOMINOR})
endif()
set(MINISATOEAS_SOVERSION ${MINISATOEAS_SOMAJOR})

#--------------------------------------------------------------------------------------------------
# Dependencies:

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})

#--------------------------------------------------------------------------------------------------
# Compile flags:

add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

#--------------------------------------------------------------------------------------------------
# Build Targets:

set(MINISATOEAS_LIB_SOURCES
    minisat/utils/Options.cc
    minisat/utils/System.cc
    minisat/core/Solver.cc
    minisat/simp/SimpSolver.cc)

add_library(minisatoeas-lib-static STATIC ${MINISATOEAS_LIB_SOURCES})
add_library(minisatoeas-lib-shared SHARED ${MINISATOEAS_LIB_SOURCES})

target_link_libraries(minisatoeas-lib-shared ${ZLIB_LIBRARY})
target_link_libraries(minisatoeas-lib-static ${ZLIB_LIBRARY})

add_executable(minisatoeas_core minisat/core/Main.cc)
add_executable(minisatoeas_simp minisat/simp/Main.cc)

if(STATIC_BINARIES)
  target_link_libraries(minisatoeas_core minisatoeas-lib-static)
  target_link_libraries(minisatoeas_simp minisatoeas-lib-static)
else()
  target_link_libraries(minisatoeas_core minisatoeas-lib-shared)
  target_link_libraries(minisatoeas_simp minisatoeas-lib-shared)
endif()

set_target_properties(minisatoeas-lib-static PROPERTIES OUTPUT_NAME "minisatoeas")
set_target_properties(minisatoeas-lib-shared
  PROPERTIES
    OUTPUT_NAME "minisatoeas"
    VERSION ${MINISATOEAS_VERSION}
    SOVERSION ${MINISATOEAS_SOVERSION})

set_target_properties(minisatoeas_simp       PROPERTIES OUTPUT_NAME "minisatoeas")

