#Set up google test so kahypar doesn't complain
set(BUILD_GTEST ON)
set(BUILD_GMOCK ON)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
target_compile_options(gmock PRIVATE -w)
target_compile_options(gtest PRIVATE -w)
include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(SYSTEM ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

#set up pybind11 or else kahypar gets angry
add_subdirectory(pybind11)

#set up kahypar including the main copy of google test, not the internal one
add_subdirectory(kahypar EXCLUDE_FROM_ALL)
target_compile_definitions(kahypar PUBLIC "LIN64")
set_target_properties(kahypar PROPERTIES PUBLIC_HEADER "lib/kahypar/include/libkahypar.h")
target_link_libraries(kahypar INTERFACE pybind11 mini_boost gtest gmock)
target_compile_options(kahypar PRIVATE -w)
target_compile_options(mini_boost PRIVATE -w)
message(STATUS ${gtest_SOURCE_DIR})
target_include_directories(kahypar INTERFACE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_include_directories(kahypar INTERFACE ${gtest_SOURCE_DIR}/../googlemock/include ${gtest_SOURCE_DIR}/../googlemock/)

#Literally everything that isn't KaHyPar goes here
add_subdirectory(mockturtle)
add_subdirectory(alice)