# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.12)

set(TOOLCHAIN_PREFIX "riscv64-unknown-elf-")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++ CACHE FILEPATH "C++ compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc CACHE FILEPATH "C compiler")

project(
  01_hello_world
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

set(CMAKE_EXECUTABLE_SUFFIX .elf)

add_executable(
  01_hello_world
  src/entry.S
  src/main.cpp
)

target_compile_features(01_hello_world PRIVATE cxx_std_23)

target_compile_options(01_hello_world PRIVATE -ffreestanding)

target_link_options(01_hello_world PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -nostdlib -z max-page-size=4096)

target_include_directories(
  01_hello_world PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
