# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

declare_arch(rv64)

set(CONFIG_DTS_PATH ${CMAKE_CURRENT_BINARY_DIR}/qemu-riscv-virt.dts)

if(NOT EXISTS ${CONFIG_DTS_PATH})
  execute_process(
    COMMAND qemu-system-riscv64 -machine virt,dumpdtb=qemu-riscv-virt.dtb -cpu rv64 -smp 1 -m 1024 -nographic -bios none
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
  execute_process(
    COMMAND dtc -o ${CONFIG_DTS_PATH} -O dts -I dtb qemu-riscv-virt.dtb
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
