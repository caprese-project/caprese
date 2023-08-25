target_sources(
  init PRIVATE
  entry.S
  startup.cpp
)

set(CONFIG_USER_PAYLOAD_BASE_ADDRESS 0xa0000000)

add_custom_target(
  init_linker
  COMMAND ${CMAKE_C_COMPILER} -DCONFIG_USER_PAYLOAD_BASE_ADDRESS=${CONFIG_USER_PAYLOAD_BASE_ADDRESS} -I ${CMAKE_BINARY_DIR}/generated -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds >${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
)
add_dependencies(init_linker apm_size)
add_dependencies(init init_linker)

target_link_options(
  init
  PRIVATE
  -T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  -nostdlib
  -z max-page-size=4096
  -e _entry
)
set_target_properties(init PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds)

set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
