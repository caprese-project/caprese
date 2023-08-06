# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

find_package(libcaprese COMPONENTS boot system_call device REQUIRED)

add_executable(kernel)

target_sources(
  kernel
  PRIVATE
  caprese/main.cpp
  caprese/panic.cpp
  caprese/memory/page.cpp
  caprese/memory/heap.cpp
  caprese/memory/page_stack.cpp
  caprese/task/task.cpp
  caprese/cpu/cpu_info.cpp
)

target_compile_features(kernel PRIVATE cxx_std_23)
target_compile_options(kernel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(
  kernel
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${ROOT_DIR}
  $<TARGET_PROPERTY:libcaprese::boot,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:libcaprese::system_call,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:libcaprese::device,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(kernel PRIVATE CONFIG_ROOT_SERVER_BIN="${CONFIG_ROOT_SERVER_BIN}" CONFIG_ROOT_SERVER_BASE_ADDRESS=${CONFIG_ROOT_SERVER_BASE_ADDRESS})

if(${CONFIG_ARCH} STREQUAL rv64)
  add_custom_command(
    OUTPUT generated_head.S
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/caprese/arch/rv64/head.S ${CMAKE_CURRENT_BINARY_DIR}/generated_head.S
    DEPENDS ${CONFIG_ROOT_TARGET}
  )

  target_sources(
    kernel
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated_head.S
    caprese/arch/rv64/boot.cpp
    caprese/arch/rv64/panic.cpp
    caprese/arch/rv64/memory/page.cpp
    caprese/arch/rv64/cpu/cpuid.cpp
    caprese/arch/rv64/task/task.cpp
    caprese/arch/rv64/task/switch.S
    caprese/arch/rv64/trap/trap.S
    caprese/arch/rv64/trap/trap.cpp
    caprese/arch/rv64/trap/system_call.cpp
    caprese/arch/rv64/libc/stdio.cpp
    caprese/arch/rv64/libc/string.cpp
    caprese/arch/rv64/libc/errno.cpp
    caprese/arch/rv64/builtins/assert_func.cpp
    caprese/arch/rv64/builtins/bswapsi2.cpp
    caprese/arch/rv64/builtins/bswapdi2.cpp
    caprese/arch/rv64/builtins/popcountdi2.cpp
  )

  target_link_options(
    kernel
    PRIVATE
    -T $<TARGET_FILE_DIR:kernel>/linker.ld
    -nostdlib
    -z max-page-size=4096
  )
  set_target_properties(kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/caprese/arch/rv64/linker.lds)
  target_link_libraries(kernel PRIVATE libcaprese::device)

  add_custom_command(
    TARGET kernel PRE_LINK
    COMMAND ${CMAKE_C_COMPILER} -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/caprese/arch/rv64/linker.lds >$<TARGET_FILE_DIR:kernel>/linker.ld
  )
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/kernel_payload
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel> ${CMAKE_BINARY_DIR}/kernel_payload
    DEPENDS kernel
  )
  add_custom_target(
    kernel_payload ALL
    DEPENDS ${CMAKE_BINARY_DIR}/kernel_payload
  )
endif()
