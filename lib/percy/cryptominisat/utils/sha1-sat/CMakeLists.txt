cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

if(NOT (CMAKE_MAJOR_VERSION LESS 2))
    if(POLICY CMP0048)
        #policy for VERSION in cmake 3.0
        cmake_policy(SET CMP0048 OLD)
    endif()
endif()

# -----------------------------------------------------------------------------
# Make RelWithDebInfo the default build type if otherwise not set
# -----------------------------------------------------------------------------
IF(DEFINED CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
  CACHE STRING "Choose the type of build: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
  "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

PROJECT(sha1-sat)

option(STATICCOMPILE "Compile to static executable" OFF)

find_package( Boost 1.46 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(sha1-gen
    main.cc
)

if (STATICCOMPILE)
    SET_TARGET_PROPERTIES(sha1-gen PROPERTIES LINK_SEARCH_START_STATIC 1)
endif()
set_target_properties(sha1-gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE)
if (STATICCOMPILE)
    SET_TARGET_PROPERTIES(sha1-gen PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()

target_link_libraries(sha1-gen
    ${Boost_LIBRARIES}
)

add_executable(sha1-verify
    verify-preimage.cc
)

