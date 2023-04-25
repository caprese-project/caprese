# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.0)

function(check_config)
  if(NOT DEFINED CONFIG_ARCH)
    message(FATAL_ERROR "CONFIG_ARCH is not defined.")
  endif()
  if(NOT ${CONFIG_ARCH} MATCHES "^(rv64)$")
    message(FATAL_ERROR "Unsupported arch: ${CONFIG_ARCH}")
  endif()

  if(NOT DEFINED CONFIG_DTS_PATH)
    message(FATAL_ERROR "CONFIG_DTS_PATH is not defined.")
  endif()
  if(NOT EXISTS ${CONFIG_DTS_PATH})
    message(FATAL_ERROR "'${CONFIG_DTS_PATH}' not found.")
  endif()
endfunction(check_config)
