target_sources(
  init
  PRIVATE
  entry.S
)

target_link_options(
  init
  PRIVATE
  -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(init PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

set(CONFIG_USER_PAYLOAD_BASE_ADDRESS 0x80000000 PARENT_SCOPE)
