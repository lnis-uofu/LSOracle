project(yosys-plugin)
add_library(yosys-plugin SHARED oracle.cc)
set_target_properties(yosys-plugin PROPERTIES OUTPUT_NAME "oracle" PREFIX "")
target_include_directories(yosys-plugin PUBLIC ${YOSYS_INCLUDE_DIR})
add_definitions(-D_YOSYS_ -DYOSYS_ENABLE_READLINE -DYOSYS_ENABLE_PLUGINS
  -DYOSYS_ENABLE_GLOB -DYOSYS_ENABLE_ZLIB -DYOSYS_ENABLE_ABC
  -DYOSYS_ENABLE_COVER)
