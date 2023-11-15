cmake_minimum_required(VERSION 3.20)

add_executable(stub)

target_sources(
  stub PRIVATE
  entry.S
)

target_link_options(
  stub
  PRIVATE
  -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(stub PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

add_custom_target(
  root_task_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stub> $<TARGET_FILE_DIR:stub>/payload
)
add_dependencies(root_task_payload stub)

set(CONFIG_ROOT_TASK_PAYLOAD $<TARGET_FILE_DIR:stub>/payload PARENT_SCOPE)
set(CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS 0xa0000000 PARENT_SCOPE)
