# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(apm)

target_sources(
  apm PRIVATE
  src/main.cpp
)

target_compile_features(apm PRIVATE cxx_std_23)
target_compile_options(apm PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(apm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(apm PRIVATE runtime)
target_link_options(apm PRIVATE -nostartfiles)

set(GEN_DIR ${CMAKE_BINARY_DIR}/generated/apm/generated)

add_custom_target(
  console_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:console> | awk '{print \"#define CONFIG_CONSOLE_SIZE \"$5}' > ${GEN_DIR}/console_size.h"
  BYPRODUCTS ${GEN_DIR}/console_size.h
  VERBATIM
)
add_dependencies(console_size console)

add_custom_target(
  shell_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:shell> | awk '{print \"#define CONFIG_SHELL_SIZE \"$5}' > ${GEN_DIR}/shell_size.h"
  BYPRODUCTS ${GEN_DIR}/shell_size.h
  VERBATIM
)
add_dependencies(shell_size shell)

add_custom_target(
  echo_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:echo> | awk '{print \"#define CONFIG_ECHO_SIZE \"$5}' > ${GEN_DIR}/echo_size.h"
  BYPRODUCTS ${GEN_DIR}/echo_size.h
  VERBATIM
)
add_dependencies(echo_size echo)


add_custom_target(
  pingpong_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:pingpong> | awk '{print \"#define CONFIG_PINGPONG_SIZE \"$5}' > ${GEN_DIR}/pingpong_size.h"
  BYPRODUCTS ${GEN_DIR}/pingpong_size.h
  VERBATIM
)
add_dependencies(pingpong_size pingpong)

add_custom_target(
  apm_payload
  COMMAND ${CMAKE_OBJCOPY} --update-section .console=$<TARGET_FILE:console> --update-section .shell=$<TARGET_FILE:shell> --update-section .echo=$<TARGET_FILE:echo> --update-section .pingpong=$<TARGET_FILE:pingpong> $<TARGET_FILE:apm> $<TARGET_FILE_DIR:apm>/payload
)
add_dependencies(apm_payload apm console shell echo pingpong)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${CONFIG_ARCH})
