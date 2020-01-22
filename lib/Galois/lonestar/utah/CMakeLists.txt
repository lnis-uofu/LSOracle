
add_subdirectory(lib)
app(utah main.cpp)
target_link_libraries(utah galois_utah)

