# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(kernel)

target_sources(
  kernel
  PRIVATE
  main.cpp
  memory/page.cpp
  memory/heap.cpp
  memory/cls.cpp
  memory/kernel_space.cpp
  task/init.cpp
  task/task.cpp
  syscall/handler.cpp
  syscall/ns_base.cpp
  syscall/ns_debug.cpp
  syscall/ns_cap.cpp
  capability/capability.cpp
  capability/builtin.cpp
)
target_compile_features(kernel PRIVATE cxx_std_23)
target_compile_options(kernel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(
  kernel
  PRIVATE
  ${ROOT_DIR}/include
  ${CMAKE_BINARY_DIR}/generated
)

target_compile_definitions(kernel PRIVATE CONFIG_USER_PAYLOAD_BASE_ADDRESS=${CONFIG_USER_PAYLOAD_BASE_ADDRESS})
target_link_libraries(kernel libc)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/arch/${CONFIG_ARCH})

add_custom_target(
  user_payload_size
  COMMAND sh "-c" "ls -l ${CONFIG_USER_PAYLOAD} | awk '{print \"#define CONFIG_USER_PAYLOAD_SIZE \"$5}' > ${GEN_DIR}/user_payload_size.h"
  BYPRODUCTS ${GEN_DIR}/user_payload_size.h
  VERBATIM
)
add_dependencies(kernel user_payload_size)

add_custom_target(
  kernel_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=${CONFIG_USER_PAYLOAD} $<TARGET_FILE:kernel> $<TARGET_FILE_DIR:kernel>/payload
)
add_dependencies(kernel_payload kernel)

add_custom_target(
  kernel_payload_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE_DIR:kernel>/payload | awk '{print \"#define CONFIG_KERNEL_PAYLOAD_SIZE \"$5}' > ${GEN_DIR}/kernel_payload_size.h"
  BYPRODUCTS ${GEN_DIR}/kernel_payload_size.h
  VERBATIM
)
add_dependencies(kernel_payload_size kernel_payload)

if(TARGET user_payload)
  add_dependencies(user_payload_size user_payload)
  add_dependencies(kernel_payload user_payload)
endif()
