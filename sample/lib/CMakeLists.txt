# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_library(lib)

target_sources(
  lib PRIVATE
  src/syscall.S
  src/capability.cpp
  src/elf.cpp
  src/debug.cpp
  src/libc/string.cpp
)

target_compile_features(lib PRIVATE cxx_std_23)
target_compile_options(lib PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(lib PUBLIC ${ROOT_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src/arch/${CONFIG_ARCH})
