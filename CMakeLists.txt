# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.12)

project(
  caprese
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

set(CMAKE_CXX_EXTENSIONS OFF)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

if(NOT DEFINED CONFIG_ROOT_SERVER_BIN)
  message(FATAL_ERROR "CONFIG_ROOT_SERVER_BIN is not defined.")
endif(NOT DEFINED CONFIG_ROOT_SERVER_BIN)

if(NOT DEFINED CONFIG_ROOT_SERVER_BASE_ADDRESS)
  message(FATAL_ERROR "CONFIG_ROOT_SERVER_BASE_ADDRESS is not defined.")
endif(NOT DEFINED CONFIG_ROOT_SERVER_BASE_ADDRESS)

add_subdirectory(${ROOT_DIR}/kernel)
add_subdirectory(${ROOT_DIR}/boot_loader)
