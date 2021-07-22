project(yosys-plugin)

if (${YOSYS_PLUGIN})
  if (NOT DEFINED YOSYS_INCLUDE_DIR)
    message(FATAL_ERROR "YOSYS_INCLUDE_DIR was not set, but plugin build was requested. If using a system installed yosys, this is typically located in /usr/local/share/yosys/include. In the yosys repository, the directory is ./share/include, or can also be specified as the root of the repository.")
  endif()
  add_subdirectory(yosys-plugin)
  if (DEFINED YOSYS_SHARE_DIR)
    install(TARGETS yosys-plugin CONFIGURATIONS RELEASE DESTINATION ${YOSYS_SHARE_DIR}/plugin)
  endif()
endif()

add_library(yosys-plugin SHARED oracle.cc)
set_target_properties(yosys-plugin PROPERTIES OUTPUT_NAME "oracle" PREFIX "")
target_include_directories(yosys-plugin PUBLIC ${YOSYS_INCLUDE_DIR})
add_definitions(-D_YOSYS_ -DYOSYS_ENABLE_READLINE -DYOSYS_ENABLE_PLUGINS
  -DYOSYS_ENABLE_GLOB -DYOSYS_ENABLE_ZLIB -DYOSYS_ENABLE_ABC
  -DYOSYS_ENABLE_COVER)
