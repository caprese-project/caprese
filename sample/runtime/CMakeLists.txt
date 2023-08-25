# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_library(runtime)

target_sources(
  runtime PRIVATE
  src/global.cpp
  src/info.cpp
  src/cap.cpp
  src/memory.cpp
  src/task.cpp
  src/syscall.S
  src/elf.cpp
)

target_compile_features(runtime PRIVATE cxx_std_23)
target_compile_options(runtime PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(runtime PUBLIC ${ROOT_DIR}/include ${SAMPLE_ROOT_DIR}/apm/include ${SAMPLE_ROOT_DIR}/console/include ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src/arch/${CONFIG_ARCH})
