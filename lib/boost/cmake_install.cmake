# Install script for directory: /home/snelgrov/nas/lsoracle/lib/boost

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/snelgrov/nas/lsoracle/lib/boost/algorithm/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/any/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/array/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/assert/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/bind/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/concept_check/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/config/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/container/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/container_hash/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/conversion/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/core/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/detail/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/dynamic_bitset/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/exception/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/function/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/function_types/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/fusion/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/integer/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/intrusive/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/io/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/iterator/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/lexical_cast/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/move/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/mpl/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/numeric_conversion/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/optional/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/predef/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/preprocessor/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/program_options/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/range/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/regex/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/smart_ptr/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/static_assert/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/throw_exception/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/tokenizer/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/tuple/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/type_index/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/typeof/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/type_traits/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/unordered/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/utility/cmake_install.cmake")
  include("/home/snelgrov/nas/lsoracle/lib/boost/winapi/cmake_install.cmake")

endif()

