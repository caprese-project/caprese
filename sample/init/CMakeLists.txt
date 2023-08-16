# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.20)

add_executable(init)

target_sources(init PRIVATE src/main.cpp)

target_compile_features(init PRIVATE cxx_std_23)
target_compile_options(init PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

target_include_directories(init PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)

target_link_libraries(init PRIVATE lib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${CONFIG_ARCH})

set(CONFIG_USER_PAYLOAD $<TARGET_FILE_DIR:init>/payload PARENT_SCOPE)
set(CONFIG_USER_PAYLOAD_BASE_ADDRESS ${CONFIG_USER_PAYLOAD_BASE_ADDRESS} PARENT_SCOPE)
