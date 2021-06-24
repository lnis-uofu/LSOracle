add_library(alice_cli11 INTERFACE) # v1.3
target_include_directories(alice_cli11 SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)

if (NOT TARGET fmt)
  add_library(fmt INTERFACE) # v6.2.1
  target_include_directories(fmt SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
  target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY)
endif()

if (NOT TARGET json)
  add_library(json INTERFACE) # v3.9.1
  target_include_directories(json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)
endif()
