# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.0)

function(declare_deps)
  if(${CONFIG_ARCH} STREQUAL rv64)
    include(ExternalProject)
    set(PREFIX ${CMAKE_BINARY_DIR}/external)
    ExternalProject_Add(
      opensbi
      PREFIX ${PREFIX}
      GIT_REPOSITORY https://github.com/riscv/opensbi
      GIT_TAG v1.2
      CONFIGURE_COMMAND ""
      BUILD_COMMAND
      make -C ${PREFIX}/src/opensbi
      CROSS_COMPILE=${TOOLCHAIN_PREFIX}
      PLATFORM=generic
      PLATFORM_RISCV_XLEN=64
      PLATFORM_RISCV_ISA=rv64gc
      PLATFORM_RISCV_ABI=lp64d
      FW_PAYLOAD_PATH=$<TARGET_FILE_DIR:boot_loader>/payload
      O=${PREFIX}/src/opensbi-build
      DEBUG=1
      INSTALL_COMMAND ""
      LOG_DOWNLOAD ON
      LOG_BUILD ON
    )
    add_custom_command(
      TARGET opensbi POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX}/src/opensbi-build/platform/generic/firmware/fw_payload.elf ${CMAKE_BINARY_DIR}/image.elf
    )
    add_dependencies(opensbi boot_loader)
  endif()
endfunction(declare_deps)
