# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/LICENSE

cmake_minimum_required(VERSION 3.12)

project(
  caprese_sample_user_servers
  VERSION 0.0.1
  HOMEPAGE_URL https://github.com/cosocaf/caprese
  LANGUAGES CXX ASM
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/root)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/console)
