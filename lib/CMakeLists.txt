#Set up google test so kahypar doesn't complain
set(BUILD_GTEST ON)
set(BUILD_GMOCK ON)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
target_compile_options(gmock PRIVATE -w)
target_compile_options(gtest PRIVATE -w)
# include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
# include_directories(SYSTEM ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

#set up pybind11 or else kahypar gets angry
add_subdirectory(pybind11)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/lib/boost/cmake)
add_subdirectory(boost)

#set up kahypar including the main copy of google test, not the internal one
set(BUILD_GTEST OFF)
set(BUILD_GMOCK OFF)
add_subdirectory(kahypar EXCLUDE_FROM_ALL)
target_compile_definitions(kahypar PUBLIC "LIN64")
target_compile_options(kahypar PRIVATE -w)

#Literally everything that isn't KaHyPar goes here
add_subdirectory(mockturtle)
add_subdirectory(alice)
if (${ENABLE_GALOIS})
  add_subdirectory(Galois EXCLUDE_FROM_ALL)
endif()
