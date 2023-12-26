cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CONFIG_ROOT_TASK_PAYLOAD)
  message("CONFIG_ROOT_TASK_PAYLOAD is not defined. Use stub as init task.")
  add_subdirectory(arch/${CONFIG_ARCH}/stub)
endif()

if(NOT DEFINED CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS)
  message(FATAL_ERROR "CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS is not defined.")
endif()

function(declare_target name)
  target_compile_features(${name} PRIVATE cxx_std_23)
  target_compile_options(${name} PRIVATE ${CONFIG_COMPILE_OPTIONS})
  target_compile_definitions(${name} PRIVATE CONFIG_BOOT_PAYLOAD_BASE=${CONFIG_BOOT_PAYLOAD_BASE} CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS=${CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS})

  target_include_directories(${name} PRIVATE ${INCLUDE_DIR})
endfunction(declare_target)

function(declare_library name type)
  add_library(${name} ${type})
  declare_target(${name})
endfunction(declare_library)

function(declare_executable name)
  add_executable(${name})
  declare_target(${name})
endfunction(declare_executable)

declare_library(caprese_kernel_libc OBJECT)

target_link_libraries(caprese_kernel_libc PUBLIC caprese_libc caprese_libcxx libcaprese)

declare_executable(caprese_kernel)
declare_executable(caprese_boot)
declare_executable(caprese_boot_stub)

target_link_libraries(caprese_kernel caprese_kernel_libc)
target_link_libraries(caprese_boot caprese_libc)
target_link_libraries(caprese_boot_stub caprese_libc)

target_sources(
  caprese_kernel PRIVATE
  kernel/cap_space.cpp
  kernel/cap.cpp
  kernel/cls.cpp
  kernel/ipc.cpp
  kernel/lock.cpp
  kernel/log.cpp
  kernel/start.cpp
  kernel/syscall.cpp
  kernel/task.cpp
  kernel/user_memory.cpp
)
add_subdirectory(arch/${CONFIG_ARCH})

add_custom_target(
  caprese_kernel_payload_base
  COMMAND sh "-c" "${CMAKE_OBJDUMP} $<TARGET_FILE:caprese_boot_stub> -h | awk '/.payload/{print \"#define CONFIG_KERNEL_PAYLOAD_BASE 0x\"$4}' > ${GENERATE_DIR}/kernel_payload_base.h"
  BYPRODUCTS ${GENERATE_DIR}/kernel_payload_base.h
  VERBATIM
)
add_dependencies(caprese_kernel_payload_base caprese_boot_stub)

add_custom_target(
  root_task_payload_size
  COMMAND sh "-c" "ls -l ${CONFIG_ROOT_TASK_PAYLOAD} | awk '{print \"#define CONFIG_ROOT_TASK_PAYLOAD_SIZE \"$5}' > ${GENERATE_DIR}/root_task_payload_size.h"
  BYPRODUCTS ${GENERATE_DIR}/root_task_payload_size.h
  VERBATIM
)
add_dependencies(caprese_kernel root_task_payload_size)

add_custom_target(
  caprese_kernel_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=${CONFIG_ROOT_TASK_PAYLOAD} $<TARGET_FILE:caprese_kernel> $<TARGET_FILE_DIR:caprese_kernel>/payload
)
add_dependencies(caprese_kernel_payload caprese_kernel)

add_custom_target(
  caprese_kernel_payload_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE_DIR:caprese_kernel>/payload | awk '{print \"#define CONFIG_KERNEL_PAYLOAD_SIZE \"$5}' > ${GENERATE_DIR}/kernel_payload_size.h"
  BYPRODUCTS ${GENERATE_DIR}/kernel_payload_size.h
  VERBATIM
)
add_dependencies(caprese_kernel_payload_size caprese_kernel_payload)

if(TARGET root_task_payload)
  add_dependencies(root_task_payload_size root_task_payload)
  add_dependencies(caprese_kernel_payload root_task_payload)
endif()

add_custom_target(
  caprese_boot_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=$<TARGET_FILE_DIR:caprese_kernel>/payload $<TARGET_FILE:caprese_boot> $<TARGET_FILE_DIR:caprese_boot>/payload
)
add_dependencies(caprese_boot_payload caprese_boot)

add_dependencies(caprese_boot caprese_kernel_payload)
