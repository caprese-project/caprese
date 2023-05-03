# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.12)

project(
  caprese_sample_root_server
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

add_executable(caprese_sample_root_server)
declare_root(caprese_sample_root_server)

target_sources(
  caprese_sample_root_server PRIVATE
  src/entry.S
  src/main.cpp
)
target_link_options(
  caprese_sample_root_server PRIVATE
  -T $<TARGET_FILE_DIR:caprese_sample_root_server>/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(caprese_sample_root_server PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.lds)

add_custom_command(
  TARGET caprese_sample_root_server PRE_LINK
  COMMAND ${CMAKE_C_COMPILER} -E -P -x c -DCONFIG_ROOT_SERVER_BASE_ADDRESS=${CONFIG_ROOT_SERVER_BASE_ADDRESS} ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.lds >$<TARGET_FILE_DIR:caprese_sample_root_server>/linker.ld
)
