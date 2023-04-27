# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.13)

add_executable(kernel)

target_sources(
  kernel
  PRIVATE
    kernel/main.cpp
    kernel/panic.cpp
    kernel/device.cpp
    kernel/page.cpp
    kernel/capability.cpp
    kernel/thread_control_block.cpp
    lib/console.cpp
    lib/string.cpp
)
target_compile_options(
  kernel PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>
)
target_include_directories(
  kernel PRIVATE
  ${ROOT_DIR}/include
)

if(${CONFIG_ARCH} STREQUAL rv64)
  target_sources(
    kernel
    PRIVATE
      arch/riscv/head.S
      arch/riscv/trap.S
      arch/riscv/arch_main.cpp
      arch/riscv/sbi.cpp
      arch/riscv/panic.cpp
  )
  target_link_options(
    kernel PRIVATE
    -T ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/linker.lds
    -nostdlib
    -z max-page-size=4096
  )
  set_target_properties(kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/linker.lds)

  add_custom_command(
    TARGET kernel POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel> $<TARGET_FILE_DIR:kernel>/payload
  )
endif()
