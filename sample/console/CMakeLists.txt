# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(console)

target_sources(
  console PRIVATE
  src/main.cpp
)

target_compile_features(console PRIVATE cxx_std_23)
target_compile_options(console PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(console PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(console PRIVATE runtime)
target_link_options(console PRIVATE -nostartfiles)
