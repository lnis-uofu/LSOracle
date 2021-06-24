include_directories(catch2) # v2.12.0

if(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wextra")
endif()

file(GLOB FILENAMES *.cpp)

add_executable(run_tests ${FILENAMES})
target_link_libraries(run_tests PUBLIC alice)
target_compile_definitions(run_tests PRIVATE ALICE_NOAPI)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wno-writable-strings" HAS_NO_WRITABLE_STRINGS)
if (HAS_NO_WRITABLE_STRINGS)
  target_compile_options(run_tests PRIVATE "-Wno-writable-strings")
endif()
check_cxx_compiler_flag("-Wno-write-strings" HAS_NO_WRITE_STRINGS)
if (HAS_NO_WRITE_STRINGS)
  target_compile_options(run_tests PRIVATE "-Wno-write-strings")
endif()
