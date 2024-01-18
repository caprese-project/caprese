cmake_minimum_required(VERSION 3.0)

function(declare_deps)
  include(FetchContent)

  set(LIBC_COMPILE_OPTIONS ${CONFIG_COMPILE_OPTIONS})
  FetchContent_Declare(
    caprese_libc
    GIT_REPOSITORY https://github.com/caprese-project/libc.git
  )
  FetchContent_GetProperties(caprese_libc)

  if(NOT caprese_libc_POPULATED)
    FetchContent_MakeAvailable(caprese_libc)
  endif()

  FetchContent_Declare(
    libcaprese
    GIT_REPOSITORY https://github.com/caprese-project/libcaprese
  )
  FetchContent_GetProperties(libcaprese)

  if(NOT libcaprese_POPULATED)
    FetchContent_MakeAvailable(libcaprese)
  endif()

  if(${CONFIG_ARCH} STREQUAL rv64)
    execute_process(
      COMMAND sh "-c" "grep cpu.cores /proc/cpuinfo | sort -u | sed 's/[^0-9]//g'"
      OUTPUT_VARIABLE NPROC
    )
    math(EXPR NPROC "${NPROC} + 1")

    set(
      MAKE_COMMAND
      make
      -j${NPROC}
      CROSS_COMPILE=${TOOLCHAIN_PREFIX}
      PLATFORM=generic
      PLATFORM_RISCV_XLEN=64
      PLATFORM_RISCV_ISA=rv64gc
      PLATFORM_RISCV_ABI=lp64d
      FW_TEXT_START=${CONFIG_FW_TEXT_START}
      FW_FDT_PATH=${CONFIG_FW_FDT_PATH}
      DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    )
    set(PAYLOAD_PATH platform/generic/firmware/fw_payload.elf)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external)

    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/external/opensbi)
      execute_process(
        COMMAND git clone https://github.com/riscv/opensbi -b v1.3.1 --depth 1
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external
      )
      execute_process(
        COMMAND ${MAKE_COMMAND} O=prebuild
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/opensbi
      )
    endif()

    execute_process(
      COMMAND sh "-c" "${CMAKE_OBJDUMP} ${CMAKE_CURRENT_BINARY_DIR}/external/opensbi/prebuild/${PAYLOAD_PATH} -h | awk '/\\.payload/{print $4}'"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/opensbi
      OUTPUT_VARIABLE PAYLOAD_BASE
    )

    math(EXPR CONFIG_BOOT_PAYLOAD_BASE "0x${PAYLOAD_BASE}" OUTPUT_FORMAT HEXADECIMAL)
    set(CONFIG_BOOT_PAYLOAD_BASE ${CONFIG_BOOT_PAYLOAD_BASE} PARENT_SCOPE)

    add_custom_target(
      opensbi ALL
      COMMAND ${MAKE_COMMAND} FW_PAYLOAD_PATH=$<TARGET_FILE_DIR:caprese_boot>/payload O=build
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external/opensbi
    )
    add_dependencies(opensbi caprese_boot_payload)

    add_custom_target(
      caprese-elf ALL
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/external/opensbi/build/${PAYLOAD_PATH} ${CMAKE_BINARY_DIR}/caprese.elf
      DEPENDS opensbi
    )

    add_custom_target(
      caprese-bin ALL
      COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_BINARY_DIR}/caprese.elf ${CMAKE_BINARY_DIR}/caprese.bin
      DEPENDS caprese-elf
    )
  endif()
endfunction(declare_deps)
