add_subdirectory(pybind11 EXCLUDE_FROM_ALL)

add_library(alice_any INTERFACE)
target_include_directories(alice_any INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/any)

add_library(alice_cli11 INTERFACE) # v1.3
target_include_directories(alice_cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)

add_library(alice_fmt INTERFACE) # v4.1.0
target_include_directories(alice_fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_compile_definitions(alice_fmt INTERFACE FMT_HEADER_ONLY)

add_library(alice_json INTERFACE) # v3.0.0
target_include_directories(alice_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)
