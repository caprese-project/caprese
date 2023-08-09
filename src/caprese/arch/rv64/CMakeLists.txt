# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

target_sources(
  kernel
  PRIVATE
  entry.S
  start.cpp
  memory.cpp
  system.cpp
  device.cpp
  task.cpp
  trap.S
  syscall.cpp
)

add_custom_target(
  linker
  COMMAND ${CMAKE_C_COMPILER} -DCONFIG_MAPPED_SPACE_BASE=${CONFIG_MAPPED_SPACE_BASE} -I ${CMAKE_BINARY_DIR}/generated -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds >${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
)
add_dependencies(linker kernel_payload_base)
add_dependencies(kernel linker)

target_link_options(
  kernel
  PRIVATE
  -T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds)

add_custom_target(
  user_payload ALL
  DEPENDS ${CONFIG_USER_PAYLOAD}
)
add_dependencies(kernel user_payload)

add_custom_target(
  kernel_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel> $<TARGET_FILE_DIR:kernel>/payload
)
add_dependencies(kernel_payload kernel)

add_custom_target(
  kernel_payload_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE_DIR:kernel>/payload | awk '{print \"#define CONFIG_KERNEL_PAYLOAD_SIZE \"$5}' > ${GEN_DIR}/kernel_payload_size.h"
  BYPRODUCTS ${GEN_DIR}/kernel_payload_size.h
  VERBATIM
)
add_dependencies(kernel_payload_size kernel_payload)
