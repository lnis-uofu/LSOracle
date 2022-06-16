
#Set up google test so kahypar doesn't complain
if (LOCAL_GTEST)
  set(BUILD_GTEST ON)
  set(BUILD_GMOCK ON)
  add_subdirectory(googletest EXCLUDE_FROM_ALL)
  target_compile_options(gmock PRIVATE -w)
  target_compile_options(gtest PRIVATE -w)
else()
  find_package(GTest)
endif()

# include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
# include_directories(SYSTEM ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

#set up pybind11 or else kahypar gets angry
if (LOCAL_PYBIND)
  add_subdirectory(pybind11)
else()
  find_package(pybind11)
endif()

if (LOCAL_BOOST)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/boost/cmake)
  add_subdirectory(boost)
else()
  find_package(Boost 1.69)
endif()

#set up kahypar including the main copy of google test, not the internal one
set(BUILD_GTEST OFF)
set(BUILD_GMOCK OFF)
add_subdirectory(kahypar EXCLUDE_FROM_ALL)
target_compile_definitions(kahypar PUBLIC "LIN64")
target_compile_options(kahypar PRIVATE -w)

#Literally everything that isn't KaHyPar goes here
if (LOCAL_FMT)
  add_subdirectory(fmt) # must go before mockturtle and alice
else()
  add_custom_target(fmt) # add this so that mockturtle detects an externally supplied version.
  find_package(fmt)
endif()

if (LOCAL_SPDLOG)
  set(SPDLOG_FMT_EXTERNAL ON)
  add_subdirectory(spdlog)
else()
  find_package(spdlog)
endif()

add_subdirectory(mockturtle)
add_subdirectory(alice)


if (${ENABLE_GALOIS})
  add_subdirectory(Galois EXCLUDE_FROM_ALL)
endif()

if (${ENABLE_ABC})

endif()

if (${ENABLE_OPENSTA})
  if (LOCAL_OPENSTA)
    add_subdirectory(OpenSTA)
  else()
    if (NOT DEFINED OPENSTA_INCLUDE_DIR OR NOT DEFINED OPENSTA_LIB_DIR)
      message(FATAL_ERROR "Either OPENSTA_INCLUDE_DIR or OPENSTA_LIB_DIR was not set, but timing integration was requested. If using a system installed OpenSTA, this is typically located in /usr/include/OpenSTA and /usr/lib/libOpenSTA.a")
    endif()
    add_library(OpenSTA STATIC IMPORTED)
    add_library(sta_swig STATIC IMPORTED)
    target_include_directories(OpenSTA INTERFACE ${OPENSTA_INCLUDE_DIR})
    target_link_libraries(OpenSTA INTERFACE ${OPENSTA_LIB_DIR})
  endif()
endif()

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)
