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
  task/init.cpp
  task/task.cpp
  capability/init.cpp
)
target_compile_features(kernel PRIVATE cxx_std_23)
target_compile_options(kernel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(
  kernel
  PRIVATE
  ${ROOT_DIR}/include
  ${CMAKE_BINARY_DIR}/generated
)

target_compile_definitions(kernel PRIVATE CONFIG_USER_PAYLOAD="${CONFIG_USER_PAYLOAD}" CONFIG_USER_PAYLOAD_BASE_ADDRESS=${CONFIG_USER_PAYLOAD_BASE_ADDRESS})
target_link_libraries(kernel libc)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/arch/${CONFIG_ARCH})
