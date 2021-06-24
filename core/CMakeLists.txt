include_directories(${PROJECT_SOURCE_DIR}/lib/kahypar/include)
add_executable(new_lso lsoracle.cpp)
target_link_libraries(new_lso alice mockturtle kahypar)