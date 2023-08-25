# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(shell)

target_sources(
  shell PRIVATE
  src/main.cpp
)

target_compile_features(shell PRIVATE cxx_std_23)
target_compile_options(shell PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(shell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(shell PRIVATE runtime)
target_link_options(shell PRIVATE -nostartfiles)
