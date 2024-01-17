# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

set(CONFIG_FW_TEXT_START 0x40000000)

declare_arch(ARCH rv64 MMU sv39)

add_compile_definitions(
  CONFIG_ANSI_ESC_SEQ=1
)
