# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

set(SAMPLE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

list(REMOVE_ITEM CONFIG_CXX_OPTIONS -ffreestanding)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/init)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/runtime)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pingpong)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/console)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shell)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/echo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dev)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fs)

add_custom_target(
  user_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary ${INIT_SECTIONS} $<TARGET_FILE:init> $<TARGET_FILE_DIR:init>/payload
)
add_dependencies(user_payload init)

set(CONFIG_USER_PAYLOAD ${CONFIG_USER_PAYLOAD} PARENT_SCOPE)
set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
