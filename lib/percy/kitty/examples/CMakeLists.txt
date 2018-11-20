# No debugging and optimization for examples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

function(add_example name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} kitty)
endfunction()

add_example(gf16_inverse gf16_inverse.cpp)
add_example(isop isop.cpp)
add_example(npn_enumeration npn_enumeration.cpp)
add_example(npn_enumeration_map npn_enumeration_map.cpp)
add_example(npn_random npn_random.cpp)
add_example(spectral_enumeration spectral_enumeration.cpp)
add_example(spectral_enumeration_file spectral_enumeration_file.cpp)
add_example(spectral_enumeration_map spectral_enumeration_map.cpp)
