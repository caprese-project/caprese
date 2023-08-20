target_sources(
  init
  PRIVATE
  entry.S
  start.cpp
)

set(CONFIG_USER_PAYLOAD_BASE_ADDRESS 0xa0000000)

add_custom_target(
  init_linker
  COMMAND ${CMAKE_C_COMPILER} -DCONFIG_USER_PAYLOAD_BASE_ADDRESS=${CONFIG_USER_PAYLOAD_BASE_ADDRESS} -I ${CMAKE_BINARY_DIR}/generated -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds >${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
)
add_dependencies(init_linker mm_size fs_size plic_size)
add_dependencies(init init_linker)

target_link_options(
  init
  PRIVATE
  -T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(init PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds)

add_custom_target(
  plic_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:plic> | awk '{print \"#define CONFIG_PLIC_SIZE \"$5}' > ${GEN_DIR}/plic_size.h"
  BYPRODUCTS ${GEN_DIR}/plic_size.h
  VERBATIM
)
add_dependencies(plic_size plic)

set(ARCH_INIT_SECTIONS --update-section .plic=$<TARGET_FILE:plic> PARENT_SCOPE)

set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
