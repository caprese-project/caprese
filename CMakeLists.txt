cmake_minimum_required(VERSION 3.12)

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

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/arch/${CONFIG_ARCH})
set(GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATE_DIR})

project(
  caprese
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/caprese-project/caprese
  LANGUAGES CXX ASM
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DCONFIG_LOG_DEBUG=1)
endif()

if(DEFINED CONFIG_LOG_LEVEL)
  string(TOLOWER "${CONFIG_LOG_LEVEL}" LOG_LEVEL)

  if(LOG_LEVEL STREQUAL all)
    add_definitions(-DCONFIG_LOG_DEBUG=1 -DCONFIG_LOG_INFO=1 -DCONFIG_LOG_WARNING=1 -DCONFIG_LOG_ERROR=1 -DCONFIG_LOG_FATAL=1)
  elseif(LOG_LEVEL STREQUAL debug)
    add_definitions(-DCONFIG_LOG_DEBUG=1 -DCONFIG_LOG_INFO=1 -DCONFIG_LOG_WARNING=1 -DCONFIG_LOG_ERROR=1 -DCONFIG_LOG_FATAL=1)
  elseif(LOG_LEVEL STREQUAL info)
    add_definitions(-DCONFIG_LOG_INFO=1 -DCONFIG_LOG_WARNING=1 -DCONFIG_LOG_ERROR=1 -DCONFIG_LOG_FATAL=1)
  elseif(LOG_LEVEL STREQUAL warning)
    add_definitions(-DCONFIG_LOG_WARNING=1 -DCONFIG_LOG_ERROR=1 -DCONFIG_LOG_FATAL=1)
  elseif(LOG_LEVEL STREQUAL error)
    add_definitions(-DCONFIG_LOG_ERROR=1 -DCONFIG_LOG_FATAL=1)
  elseif(LOG_LEVEL STREQUAL fatal)
    add_definitions(-DCONFIG_LOG_FATAL=1)
  else()
    message(WARNING "Unknown log level: ${CONFIG_LOG_LEVEL}")
  endif()
endif()

add_subdirectory(src)
