#!/usr/bin/cmake -P
#
# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.0)

execute_process(
  COMMAND
    qemu-system-riscv64
      -M virt
      -cpu rv64
      -smp 1
      -m 2048
      -nographic
      -bios build/image.elf
      # -gdb tcp::1234
      # -S
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
