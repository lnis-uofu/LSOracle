file(GLOB FILENAMES *.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_BUILD_TYPE "Release")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL 
    "Enable testing of the benchmark library." FORCE)

add_subdirectory(benchmark)

foreach(FILENAME ${FILENAMES})
    get_filename_component(BASENAME ${FILENAME} NAME_WE)

    set(TARGET_NAME ${BASENAME})

    add_executable(${TARGET_NAME} ${FILENAME})
    target_link_libraries(${TARGET_NAME} percy benchmark)
endforeach()


