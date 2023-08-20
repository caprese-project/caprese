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
  src/early_memory.cpp
)

target_compile_features(init PRIVATE cxx_std_23)
target_compile_options(init PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(init PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/lib/include)

target_link_libraries(init PRIVATE lib)

add_custom_target(
  mm_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:mm> | awk '{print \"#define CONFIG_MM_SIZE \"$5}' > ${GEN_DIR}/mm_size.h"
  BYPRODUCTS ${GEN_DIR}/mm_size.h
  VERBATIM
)
add_dependencies(mm_size mm)

add_custom_target(
  fs_size
  COMMAND sh "-c" "ls -l $<TARGET_FILE:fs> | awk '{print \"#define CONFIG_FS_SIZE \"$5}' > ${GEN_DIR}/fs_size.h"
  BYPRODUCTS ${GEN_DIR}/fs_size.h
  VERBATIM
)
add_dependencies(fs_size fs)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${CONFIG_ARCH})

set(INIT_SECTIONS --update-section .mm=$<TARGET_FILE:mm> --update-section .fs=$<TARGET_FILE:fs> ${ARCH_INIT_SECTIONS} PARENT_SCOPE)

set(CONFIG_USER_PAYLOAD $<TARGET_FILE_DIR:init>/payload PARENT_SCOPE)
set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
