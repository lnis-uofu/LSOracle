project(lsoracle)

find_package(Git)
if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --verify --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE LSORACLE_GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  set(LSORACLE_GIT_REVISION "UNKNOWN")
endif()
set(LSORACLE_VERSION "2.0.0")
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  COMMAND python3 ${PROJECT_SOURCE_DIR}/generate_kahypar.py ${PROJECT_SOURCE_DIR}/test.ini ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/test.ini
)

# "We do not recommend using GLOB to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed then the generated
# build system cannot know when to ask CMake to regenerate."
# https://cmake.org/cmake/help/latest/command/file.html
set(LSORACLE_SOURCE
  ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  kahypar_temp_config.cpp
  algorithms/optimization/resynthesis.cpp
  utility.cpp
)

add_executable(lsoracle lsoracle.cpp ${LSORACLE_SOURCE})
target_include_directories(lsoracle PRIVATE .)
target_include_directories(lsoracle PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lsoracle alice mockturtle kahypar nlohmann_json::nlohmann_json)

if (${ENABLE_GALOIS})
  add_definitions(-DENABLE_GALOIS)
  target_include_directories(lsoracle PRIVATE ../lib/Galois/lonestar)
  target_link_libraries(lsoracle galois_utah)
endif()

set(LSORACLE_TESTS
  algorithms/partitioning/__tests__/sap_test.cpp
  algorithms/partitioning/__tests__/partition_manager_junior_test.cpp
)
add_executable(unit_tests ${LSORACLE_SOURCE} ${LSORACLE_TESTS})
target_link_libraries(unit_tests gtest_main alice mockturtle kahypar)
target_include_directories(unit_tests INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0 OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
  target_link_libraries(lsoracle stdc++fs)
  target_link_libraries(unit_tests stdc++fs)
endif()

if (${ENABLE_ABC})
  add_definitions(-DENABLE_ABC)
  target_include_directories(lsoracle PRIVATE ${ABC_INCLUDE_DIR})
  # target_link_libraries(lsoracle libabc)
endif()

if (${ENABLE_OPENSTA})
  add_definitions(-DENABLE_OPENSTA)
  find_package(TCL)
  include(FindZLIB)
  include_directories (${TCL_INCLUDE_PATH})
  # add_definitions (-DUSE_TCL_STUBS)
  target_link_libraries(lsoracle OpenSTA sta_swig ${TCL_LIBRARY} ${ZLIB_LIBRARIES})
  target_link_libraries(unit_tests OpenSTA sta_swig ${TCL_LIBRARY} ${ZLIB_LIBRARIES})
endif()

enable_testing()
include(GoogleTest)
gtest_discover_tests(unit_tests)

configure_file(test.ini test.ini COPYONLY)
install(TARGETS lsoracle CONFIGURATIONS RELEASE RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES test.ini CONFIGURATIONS RELEASE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/lsoracle)
