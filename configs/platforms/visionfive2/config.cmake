# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

set(CONFIG_FW_TEXT_START 0x40000000)
set(CONFIG_FW_FDT_PATH ${CMAKE_CURRENT_BINARY_DIR}/visionfive2.dtb)
set(CONFIG_MAX_DEVICE_REGIONS 0x88)

execute_process(
  COMMAND sh "-c" "dtc -I dts -O dtb -o ${CMAKE_CURRENT_BINARY_DIR}/visionfive2.dtb ${CMAKE_CURRENT_LIST_DIR}/visionfive2.dts"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

declare_arch(ARCH rv64 MMU sv39)

add_compile_definitions(
  CONFIG_ANSI_ESC_SEQ=1
)

add_custom_target(
  fit-image
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/visionfive2-fit-image.its ${CMAKE_CURRENT_BINARY_DIR}/visionfive2-fit-image.its
)

add_custom_target(
  caprese-img ALL
  COMMAND sh "-c" "mkimage -f ${CMAKE_CURRENT_BINARY_DIR}/visionfive2-fit-image.its -A riscv -O u-boot -T firmware ${CMAKE_BINARY_DIR}/caprese.img"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS fit-image
  DEPENDS caprese-bin
)
