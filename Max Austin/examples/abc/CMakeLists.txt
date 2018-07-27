add_executable(abc2 abc.cpp)
target_link_libraries(abc2 alice lorina kitty libabc)

#Python extension doesn't work with c++17
#add_alice_python_module(abc2 abc.cpp)
#target_link_libraries(abc2_python PRIVATE libabc lorina kitty)

add_alice_c_library(abc2 abc.cpp)
target_link_libraries(abc2_c PRIVATE libabc lorina kitty)
