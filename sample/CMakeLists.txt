# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/init)

add_custom_target(
  user_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:init> $<TARGET_FILE_DIR:init>/payload
)
add_dependencies(user_payload init)

set(CONFIG_USER_PAYLOAD ${CONFIG_USER_PAYLOAD} PARENT_SCOPE)
set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
