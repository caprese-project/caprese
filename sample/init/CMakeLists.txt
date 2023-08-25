# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(init)

target_sources(
  init PRIVATE
  src/main.cpp
)

target_compile_features(init PRIVATE cxx_std_23)
target_compile_options(init PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}> -Wno-error=main)

target_include_directories(init PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(init runtime)

set(GEN_DIR ${CMAKE_BINARY_DIR}/generated/init/generated)

add_custom_target(
  apm_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:apm> | awk '{print \"#define CONFIG_APM_SIZE \"$5}' > ${GEN_DIR}/apm_size.h"
  BYPRODUCTS ${GEN_DIR}/apm_size.h
  VERBATIM
)
add_dependencies(apm_size apm)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${CONFIG_ARCH})

add_dependencies(init apm_payload)
set(INIT_SECTIONS --update-section .apm=$<TARGET_FILE_DIR:apm>/payload ${ARCH_INIT_SECTIONS} PARENT_SCOPE)

set(CONFIG_USER_PAYLOAD $<TARGET_FILE_DIR:init>/payload PARENT_SCOPE)
set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
