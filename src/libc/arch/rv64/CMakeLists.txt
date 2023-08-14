# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

add_library(libc)

target_sources(
  libc
  PRIVATE
  errno.cpp
  stdio.cpp
  stdlib.cpp
  string.cpp
  builtin/assert_func.cpp
  builtin/bswapsi2.cpp
  builtin/bswapdi2.cpp
  builtin/ctzdi2.cpp
  builtin/popcountdi2.cpp
)

target_compile_features(libc PRIVATE cxx_std_23)
target_compile_options(libc PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)
target_include_directories(libc PRIVATE ${ROOT_DIR}/include)
