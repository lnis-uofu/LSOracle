add_library(oracle INTERFACE)
target_include_directories(oracle INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(oracle INTERFACE oracle_mockturtle)
