cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CONFIG_ROOT_TASK_PAYLOAD)
  message("CONFIG_ROOT_TASK_PAYLOAD is not defined. Use stub as init task.")
  add_subdirectory(arch/${CONFIG_ARCH}/stub)
endif()

if(NOT DEFINED CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS)
  message(FATAL_ERROR "CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS is not defined.")
endif()

set(COMMON_LIBRARIES "")

function(declare_target name)
  target_compile_features(${name} PRIVATE cxx_std_23)
  target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:C>:${CONFIG_C_OPTIONS}> $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)
  target_compile_definitions(${name} PRIVATE CONFIG_BOOT_PAYLOAD_BASE=${CONFIG_BOOT_PAYLOAD_BASE} CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS=${CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS})

  target_include_directories(${name} PRIVATE ${INCLUDE_DIR})
endfunction(declare_target)

function(declare_library name common)
  add_library(${name} STATIC)
  declare_target(${name})

  if(common)
    set(COMMON_LIBRARIES "${COMMON_LIBRARIES}" ${name} PARENT_SCOPE)
  endif()
endfunction(declare_library)

function(declare_executable name)
  add_executable(${name})
  declare_target(${name})
  target_link_libraries(${name} ${COMMON_LIBRARIES})
endfunction(declare_executable)

declare_library(libc TRUE)
declare_library(log TRUE)

target_sources(
  log PRIVATE
  log/log.cpp
)

target_link_libraries(caprese_libc_stdio PUBLIC libc)
target_link_libraries(caprese_libc_signal PUBLIC libc)
target_link_libraries(libc PUBLIC caprese_libc caprese_libcxx)
target_link_libraries(log PUBLIC caprese_libc caprese_libcxx)

declare_executable(boot_stub)
declare_executable(kernel)
declare_executable(boot)

target_sources(
  kernel PRIVATE
  kernel/cls.cpp
  kernel/lock.cpp
  kernel/start.cpp
  kernel/syscall.cpp
  kernel/task.cpp
)
add_subdirectory(arch/${CONFIG_ARCH})

add_custom_target(
  kernel_payload_base
  COMMAND sh "-c" "${CMAKE_OBJDUMP} $<TARGET_FILE:boot_stub> -h | awk '/.payload/{print \"#define CONFIG_KERNEL_PAYLOAD_BASE 0x\"$4}' > ${GENERATE_DIR}/kernel_payload_base.h"
  BYPRODUCTS ${GENERATE_DIR}/kernel_payload_base.h
  VERBATIM
)
add_dependencies(kernel_payload_base boot_stub)

add_custom_target(
  root_task_payload_size
  COMMAND sh "-c" "ls -l ${CONFIG_ROOT_TASK_PAYLOAD} | awk '{print \"#define CONFIG_ROOT_TASK_PAYLOAD_SIZE \"$5}' > ${GENERATE_DIR}/root_task_payload_size.h"
  BYPRODUCTS ${GENERATE_DIR}/root_task_payload_size.h
  VERBATIM
)
add_dependencies(kernel root_task_payload_size)

add_custom_target(
  kernel_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=${CONFIG_ROOT_TASK_PAYLOAD} $<TARGET_FILE:kernel> $<TARGET_FILE_DIR:kernel>/payload
)
add_dependencies(kernel_payload kernel)

add_custom_target(
  kernel_payload_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE_DIR:kernel>/payload | awk '{print \"#define CONFIG_KERNEL_PAYLOAD_SIZE \"$5}' > ${GENERATE_DIR}/kernel_payload_size.h"
  BYPRODUCTS ${GENERATE_DIR}/kernel_payload_size.h
  VERBATIM
)
add_dependencies(kernel_payload_size kernel_payload)

if(TARGET root_task_payload)
  add_dependencies(root_task_payload_size root_task_payload)
  add_dependencies(kernel_payload root_task_payload)
endif()

add_custom_target(
  boot_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=$<TARGET_FILE_DIR:kernel>/payload $<TARGET_FILE:boot> $<TARGET_FILE_DIR:boot>/payload
)
add_dependencies(boot_payload boot)

add_dependencies(boot kernel_payload)
