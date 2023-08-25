# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

target_sources(
  runtime PRIVATE
  entry.s
  startup.cpp
  libc/assert.cpp
  libc/stdio.cpp
  libc/stdlib.cpp
  libc/string.cpp
)
