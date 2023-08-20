cmake_minimum_required(VERSION 3.20)

if(CONFIG_ARCH STREQUAL "rv64")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plic)
endif()
