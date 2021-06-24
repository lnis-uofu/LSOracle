# library for stand-alone executables
add_library(alice INTERFACE)
target_include_directories(alice SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(alice INTERFACE alice_cli11)
target_link_libraries(alice INTERFACE fmt)
target_link_libraries(alice INTERFACE json)

# library for Python bindings
add_library(alice_python INTERFACE)
target_link_libraries(alice_python INTERFACE alice)
target_compile_definitions(alice_python INTERFACE ALICE_PYTHON)
target_link_libraries(alice_python INTERFACE pybind11::module)

# library for C interface
add_library(alice_cinterface INTERFACE)
target_link_libraries(alice_cinterface INTERFACE alice)
target_compile_definitions(alice_cinterface INTERFACE ALICE_CINTERFACE)

if(ALICE_READLINE_LIBRARY STREQUAL "READLINE")
  include(CheckCXXSourceRuns)
  set(CMAKE_REQUIRED_LIBRARIES readline)
  check_cxx_source_runs("
#include <stdio.h>
#include <readline/readline.h>
void foo() { readline(\"foo\"); }
int main() { return 0; }
" READLINE_WORKS)
  unset(CMAKE_REQUIRED_LIBRARIES)

  if( READLINE_WORKS )
    target_compile_definitions(alice INTERFACE "READLINE_USE_READLINE")
    target_link_libraries(alice INTERFACE readline)
  else()
    message(WARNING "Cannot find readline library, fall back to native interface.")
  endif()
endif()

# disable some warnings
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wno-unneeded-internal-declaration" HAS_NO_UNNEEDED_INTERNAL_DECLARATION)
if (HAS_NO_UNNEEDED_INTERNAL_DECLARATION)
  target_compile_options(alice INTERFACE "-Wno-unneeded-internal-declaration")
endif()
