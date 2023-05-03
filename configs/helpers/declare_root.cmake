# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.15)

macro(declare_root root_target)
  set(CONFIG_ROOT_TARGET ${root_target} CACHE STRING "root server target")
  set(CONFIG_ROOT_SERVER_BIN $<TARGET_FILE_DIR:${root_target}>/$<TARGET_FILE_BASE_NAME:${root_target}>.bin CACHE INTERNAL "root server binary")
  set(CONFIG_ROOT_SERVER_BASE_ADDRESS 0x80000000 CACHE STRING "base address of root server")

  add_custom_command(
    TARGET ${CONFIG_ROOT_TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CONFIG_ROOT_TARGET}> ${CONFIG_ROOT_SERVER_BIN}
  )
endmacro(declare_root)
