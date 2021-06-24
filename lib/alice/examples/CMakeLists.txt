# No debugging and optimization for examples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

add_executable(demo demo.cpp)
target_link_libraries(demo PUBLIC alice)
target_compile_definitions(demo PUBLIC ALICE_ENV_HOME=ALICE_HOME)
add_alice_python_module(demo demo.cpp)
add_alice_c_library(demo demo.cpp)

# tutorial 1
add_executable(tutorial1 tutorial1/tutorial1.cpp)
target_link_libraries(tutorial1 PUBLIC alice)

# tutorial 2
add_executable(tutorial2 tutorial2/tutorial2.cpp)
target_link_libraries(tutorial2 PUBLIC alice)

# tutorial 3
add_executable(tutorial3 tutorial3/tutorial3.cpp)
target_link_libraries(tutorial3 PUBLIC alice)

# tutorial 4
add_executable(tutorial4 tutorial4/tutorial4.cpp)
target_link_libraries(tutorial4 PUBLIC alice)

# tutorial 5
add_executable(tutorial5 tutorial5/tutorial5.cpp)
target_link_libraries(tutorial5 PUBLIC alice)
