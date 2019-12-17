include(GNUInstallDirs)

#alterei pra static e alterei o target include pra public
add_library(kahypar SHARED libkahypar.cc)

target_link_libraries(kahypar LINK_PUBLIC ${Boost_LIBRARIES})

set_target_properties(kahypar PROPERTIES
        PUBLIC_HEADER ../include/libkahypar.h)

target_include_directories(kahypar PRIVATE ../include)

configure_file(libkahypar.pc.in libkahypar.pc @ONLY)

set (CMAKE_INSTALL_PREFIX=../include)

#if(WIN32)
#       install(TARGETS kahypar
#               RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
#               PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#else()
#       install(TARGETS kahypar
#               LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#               PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#endif()

#install(TARGETS kahypar LIBRARY DESTINATION ../include
#                               PUBLIC_HEADER DESTINATION ../include)

install(TARGETS kahypar
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#adicionei esse comando
install(FILES ${CMAKE_BINARY_DIR}/lib/libkahypar.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

configure_file(cmake_uninstall.cmake.in cmake_uninstall.cmake IMMEDIATE @ONLY)

add_custom_target(uninstall-kahypar "${CMAKE_COMMAND}" -P cmake_uninstall.cmake)
