file(GLOB FILENAMES *.cpp)

# No debugging and optimization for benchmark test
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
set(CMAKE_BUILD_TYPE "Release")
add_subdirectory(benchmark)

foreach(FILENAME ${FILENAMES})
  get_filename_component(BASENAME ${FILENAME} NAME_WE)

  set(TARGET_NAME bench_${BASENAME})

  add_executable(${TARGET_NAME} ${FILENAME})
  target_link_libraries(${TARGET_NAME} benchmark kitty)
  target_compile_options(${TARGET_NAME} PRIVATE -O2 -DNDEBUG)
endforeach()

