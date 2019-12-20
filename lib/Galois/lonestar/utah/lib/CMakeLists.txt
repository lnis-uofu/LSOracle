set(sources 
  $<TARGET_OBJECTS:galois_shmem_obj> 
  $<TARGET_OBJECTS:gllvm_obj>
  ../Coarsening.cpp
  ../BiPart.cpp
  ../Refine.cpp
  ../Partitioning.cpp
  ../Metric.cpp
  
)

target_link_libraries(${name} lonestar)
add_library(galois_utah STATIC ${sources})

target_link_libraries(galois_utah galois_shmem gllvm lonestar)

target_include_directories(galois_utah PUBLIC
  ${CMAKE_SOURCE_DIR}/libllvm/include
  ${CMAKE_SOURCE_DIR}/libgalois/include
  ${CMAKE_SOURCE_DIR}/lonestar/utah
  ${CMAKE_SOURCE_DIR}/lonestar/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  
)

set_target_properties (galois_utah PROPERTIES
  INTERFACE_POSITION_INDEPENDENT_CODE On
  POSITION_INDEPENDENT_CODE On
)
