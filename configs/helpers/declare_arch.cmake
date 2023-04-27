# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.12)

macro(declare_arch arch)
  if(${arch} STREQUAL rv64)
    set(CONFIG_ARCH rv64)

    set(TOOLCHAIN_PREFIX "riscv64-unknown-elf-")
    set(CMAKE_EXECUTABLE_SUFFIX .elf)

    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)

    set(
      CONFIG_CXX_OPTIONS
      -Wall
      -Werror
      -O0
      -fno-omit-frame-pointer
      -ggdb
      -gdwarf-2
      -MD
      -mcmodel=medany
      -ffreestanding
      -fno-common
      -mno-relax
      -fno-stack-protector
      -fno-exceptions
      -fno-pie
      -no-pie
    )

    add_compile_definitions(
      $<$<COMPILE_LANGUAGE:CXX>:CONFIG_ARCH_RISCV>
      $<$<COMPILE_LANGUAGE:CXX>:CONFIG_ARCH_RISCV64>
    )
  else()
    message(FATAL_ERROR "Unsupported arch: ${arch}")
  endif()
endmacro(declare_arch)
