# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_library(lib)

target_include_directories(lib PUBLIC ${ROOT_DIR}/include)

add_subdirectory(src/arch/${CONFIG_ARCH})
