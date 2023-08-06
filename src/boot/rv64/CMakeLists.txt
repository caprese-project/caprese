# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.16)

find_package(libcaprese COMPONENTS device util REQUIRED)

add_executable(boot_loader)

target_sources(
  boot_loader
  PRIVATE
  src/entry.S
  src/start.cpp
  src/memory.cpp
  src/capability.cpp
  src/libc.cpp
)

set_source_files_properties(src/entry.S OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/kernel_payload)

target_compile_features(boot_loader PRIVATE cxx_std_23)
target_compile_options(boot_loader PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_compile_definitions(boot_loader PRIVATE CONFIG_KERNEL_BIN="${CMAKE_BINARY_DIR}/kernel_payload")

target_include_directories(
  boot_loader
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${ROOT_DIR}/kernel/include
  $<TARGET_PROPERTY:libcaprese::device,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:libcaprese::util,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_options(
  boot_loader
  PRIVATE
  -T $<TARGET_FILE_DIR:boot_loader>/linker.ld
  -nostdlib
  -z max-page-size=4096
)
set_target_properties(boot_loader PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.lds)

target_link_libraries(boot_loader PRIVATE libcaprese::device libcaprese::util)

add_dependencies(boot_loader kernel_payload)

add_custom_command(
  TARGET boot_loader PRE_LINK
  COMMAND ${CMAKE_C_COMPILER} -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.lds >$<TARGET_FILE_DIR:boot_loader>/linker.ld
)
add_custom_command(
  TARGET boot_loader POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:boot_loader> $<TARGET_FILE_DIR:boot_loader>/payload
)
