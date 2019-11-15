# Build a Python extension module:
# add_alice_python_module(<name> source1 [source2 ...])
#
function(add_alice_python_module target_name)
  cmake_parse_arguments(ARG "" "" "" ${ARGN})
  add_library("${target_name}_python" MODULE ${ARG_UNPARSED_ARGUMENTS})
  target_link_libraries("${target_name}_python" PRIVATE alice_python)
  set_target_properties("${target_name}_python" PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" OUTPUT_NAME ${target_name} SUFFIX "${PYTHON_MODULE_EXTENSION}")
endfunction()

function(add_alice_c_library target_name)
  cmake_parse_arguments(ARG "" "" "" ${ARGN})
  add_library("${target_name}_c" SHARED ${ARG_UNPARSED_ARGUMENTS})
  target_link_libraries("${target_name}_c" PRIVATE alice_cinterface)
endfunction()
