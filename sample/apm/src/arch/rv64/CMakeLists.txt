target_sources(
  apm PRIVATE
  sections.s
)

add_custom_target(
  apm_linker
  COMMAND ${CMAKE_C_COMPILER} -DCONFIG_USER_PAYLOAD_BASE_ADDRESS=${CONFIG_USER_PAYLOAD_BASE_ADDRESS} -I ${CMAKE_BINARY_DIR}/generated -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds >${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
)
add_dependencies(apm_linker console_size shell_size echo_size)
add_dependencies(apm apm_linker)

target_link_options(
  apm
  PRIVATE
  -T ${CMAKE_CURRENT_BINARY_DIR}/linker.ld
  -z max-page-size=4096
)
set_target_properties(apm PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds)
