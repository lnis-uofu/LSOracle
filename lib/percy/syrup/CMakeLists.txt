file(GLOB core_srcs core/*.cc)
file(GLOB simp_srcs simp/*.cc)
file(GLOB parr_srcs parallel/*.cc)
file(GLOB util_srcs utils/*.cc)

list(FILTER simp_srcs EXCLUDE REGEX ".*Main.cc$")
list(FILTER parr_srcs EXCLUDE REGEX ".*Main.cc$")

add_library(syrup ${core_srcs} ${simp_srcs} ${parr_srcs} ${util_srcs})

target_include_directories(syrup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(syrup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
