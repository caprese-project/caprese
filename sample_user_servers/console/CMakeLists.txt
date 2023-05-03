# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.12)

project(
  caprese_sample_console_server
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

add_executable(caprese_sample_console_server)

target_sources(
  caprese_sample_console_server PRIVATE
  src/main.cpp
)
