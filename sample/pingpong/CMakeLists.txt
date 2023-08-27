# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(pingpong)

target_sources(
  pingpong PRIVATE
  src/main.cpp
)

target_compile_features(pingpong PRIVATE cxx_std_23)
target_compile_options(pingpong PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}> -Wno-error=main)

target_include_directories(pingpong PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(pingpong PRIVATE runtime)
target_link_options(pingpong PRIVATE -nostartfiles)
