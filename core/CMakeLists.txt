project(lsoracle)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  COMMAND python3 ${PROJECT_SOURCE_DIR}/generate_kahypar.py ${PROJECT_SOURCE_DIR}/test.ini ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/test.ini
)

message(WARNING ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lsoracle lsoracle.cpp ${CMAKE_CURRENT_BINARY_DIR}/kahypar_config.cpp kahypar_temp_config.cpp)
target_include_directories(lsoracle PRIVATE .)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(liblsoracle lsoracle.cpp)
target_include_directories(liblsoracle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
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
install(TARGETS lsoracle CONFIGURATIONS RELEASE RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES core/test.ini CONFIGURATIONS RELEASE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/lsoracle)
