cmake_minimum_required(VERSION 3.20)

target_sources(
  libc PRIVATE
  libc/errno.cpp
  libc/stdio.cpp
  libc/stdlib.cpp
)

target_sources(
  log PRIVATE
  log/dump.cpp
)

function(declare_boot name)
  target_sources(
    ${name} PRIVATE
    boot/entry.S
    boot/start.cpp
  )

  add_custom_target(
    ${name}_linker
    COMMAND ${CMAKE_C_COMPILER} -DCONFIG_BOOT_PAYLOAD_BASE=${CONFIG_BOOT_PAYLOAD_BASE} -I ${GENERATE_DIR} -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/boot/linker.ldS >${CMAKE_CURRENT_BINARY_DIR}/boot/${name}-linker.ld
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/boot/${name}-linker.ld
  )
  add_dependencies(${name} ${name}_linker)

  target_link_options(
    ${name}
    PRIVATE
    -T ${CMAKE_CURRENT_BINARY_DIR}/boot/${name}-linker.ld
    -nostdlib
    -z max-page-size=4096
  )
  set_target_properties(${name} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot/linker.ldS)
endfunction(declare_boot)

declare_boot(boot_stub)
declare_boot(boot)
add_dependencies(boot_linker kernel_payload_size)

target_sources(
  kernel PRIVATE
  kernel/core_id.cpp
  kernel/entry.S
  kernel/start.cpp
  kernel/trap.cpp
  kernel/trap.S
)

add_custom_target(
  kernel_linker
  COMMAND ${CMAKE_C_COMPILER} -DCONFIG_MAPPED_SPACE_BASE=${CONFIG_MAPPED_SPACE_BASE} -I ${GENERATE_DIR} -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ldS >${CMAKE_CURRENT_BINARY_DIR}/kernel/linker.ld
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ldS
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/kernel/linker.ld
)
add_dependencies(kernel_linker kernel_payload_base)
add_dependencies(kernel kernel_linker)

target_link_options(
  kernel
  PRIVATE
  -T ${CMAKE_CURRENT_BINARY_DIR}/kernel/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ldS)
