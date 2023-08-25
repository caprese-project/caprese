# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(echo)

target_sources(
  echo PRIVATE
  src/main.cpp
)

target_compile_features(echo PRIVATE cxx_std_23)
target_compile_options(echo PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(echo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${SAMPLE_ROOT_DIR}/runtime/include)

target_link_libraries(echo PRIVATE runtime)
target_link_options(echo PRIVATE -nostartfiles)
