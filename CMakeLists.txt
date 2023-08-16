# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_EXTENSIONS OFF)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GEN_DIR ${CMAKE_BINARY_DIR}/generated/caprese/generated)
file(MAKE_DIRECTORY ${GEN_DIR})

include(configs/helpers/declare_arch.cmake)
include(configs/helpers/check_config.cmake)
include(configs/helpers/declare_deps.cmake)

if(DEFINED PLATFORM)
  include(configs/platforms/${PLATFORM}/config.cmake)
  check_config()
else()
  message(FATAL_ERROR "PLATFORM is not defined.")
endif(DEFINED PLATFORM)

declare_deps()

project(
  caprese
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

if(NOT DEFINED CONFIG_USER_PAYLOAD OR NOT DEFINED CONFIG_USER_PAYLOAD_BASE_ADDRESS)
  add_subdirectory(${ROOT_DIR}/sample)

  if(NOT DEFINED CONFIG_USER_PAYLOAD OR NOT DEFINED CONFIG_USER_PAYLOAD_BASE_ADDRESS)
    message(FATAL_ERROR "Invalid configuration.")
  endif()
endif()

add_subdirectory(${ROOT_DIR}/src)
