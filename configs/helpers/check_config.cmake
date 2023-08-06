# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

function(check_config)
  if(NOT DEFINED CONFIG_ARCH)
    message(FATAL_ERROR "CONFIG_ARCH is not defined.")
  endif()

  if(NOT ${CONFIG_ARCH} MATCHES "^(rv64)$")
    message(FATAL_ERROR "Unsupported arch: ${CONFIG_ARCH}")
  endif()
endfunction(check_config)
