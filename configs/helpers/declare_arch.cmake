# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.12)

macro(declare_arch)
  cmake_parse_arguments(DECLARE_ARCH "" "ARCH;MMU" "" ${ARGN})

  if(${DECLARE_ARCH_ARCH} STREQUAL rv64)
    if(NOT DEFINED DECLARE_ARCH_MMU)
      message(FATAL_ERROR "The argument MMU is required.")
    endif()

    if(NOT ${DECLARE_ARCH_MMU} MATCHES "^(sv39|sv48)$")
      message(FATAL_ERROR "Unsupported MMU: ${DECLARE_ARCH_MMU}")
    endif()

    set(CONFIG_ARCH ${DECLARE_ARCH_ARCH})
    set(CONFIG_MMU ${DECLARE_ARCH_MMU})

    set(TOOLCHAIN_PREFIX "riscv64-unknown-elf-")
    set(CMAKE_EXECUTABLE_SUFFIX .elf)

    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc CACHE FILEPATH "C compiler")
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++ CACHE FILEPATH "C++ compiler")
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc CACHE FILEPATH "C compiler")
    set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE STRING "objcopy")
    set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE STRING "objdump")

    set(
      CONFIG_C_OPTIONS
      -Wall
      -Wextra
      -Werror
      -Wno-format-overflow
      -Wno-attributes
      -fno-omit-frame-pointer
      -gdwarf-2
      -MD
      -mcmodel=medany
      -ffreestanding
      -fno-common
      -mno-relax
      -fno-stack-protector
      -fno-exceptions
      -fno-pie
      -no-pie
      $<IF:$<CONFIG:Debug>,-O0,-O3>
      $<IF:$<CONFIG:Debug>,-g3,-g0>
    )

    set(
      CONFIG_CXX_OPTIONS
      -Wall
      -Wextra
      -Werror
      -Wno-format-overflow
      -Wno-attributes
      -fno-omit-frame-pointer
      -gdwarf-2
      -MD
      -mcmodel=medany
      -ffreestanding
      -fno-common
      -mno-relax
      -fno-stack-protector
      -fno-exceptions
      -fno-pie
      -no-pie
      -fno-use-cxa-atexit
      $<IF:$<CONFIG:Debug>,-O0,-O3>
      $<IF:$<CONFIG:Debug>,-g3,-g0>
    )

    add_compile_definitions(
      CONFIG_ARCH_RISCV
      CONFIG_XLEN_64
    )

    if(NOT DEFINED CONFIG_MAX_CORES)
      set(CONFIG_MAX_CORES 32)
    endif()

    if(NOT DEFINED CONFIG_MAX_RESERVED_REGIONS)
      # mmode_resv0(OpenSBI), mmode_resv1(OpenSBI), kernel, payload
      set(CONFIG_MAX_RESERVED_REGIONS 4)
    endif()

    if(NOT DEFINED CONFIG_MAX_DEVICE_REGIONS)
      set(CONFIG_MAX_DEVICE_REGIONS 32)
    endif()

    if(NOT DEFINED CONFIG_MAX_MEMORY_REGIONS)
      set(CONFIG_MAX_MEMORY_REGIONS 1)
    endif()

    if(NOT DEFINED CONFIG_MAX_TASKS)
      math(EXPR CONFIG_MAX_TASKS "1 << 22" OUTPUT_FORMAT HEXADECIMAL)
    endif()

    if(${CONFIG_MMU} STREQUAL "sv39")
      add_compile_definitions(CONFIG_MMU_SV39)
      math(EXPR CONFIG_MAX_VIRTUAL_ADDRESS "1 << 38" OUTPUT_FORMAT HEXADECIMAL)
      math(EXPR CONFIG_MAX_PHYSICAL_ADDRESS "1 << 36" OUTPUT_FORMAT HEXADECIMAL)
    elseif(${CONFIG_MMU} STREQUAL "sv48")
      add_compile_definitions(CONFIG_MMU_SV48)
      math(EXPR CONFIG_MAX_VIRTUAL_ADDRESS "1 << 47" OUTPUT_FORMAT HEXADECIMAL)
      math(EXPR CONFIG_MAX_PHYSICAL_ADDRESS "1 << 45" OUTPUT_FORMAT HEXADECIMAL)
    endif()

    math(EXPR CONFIG_USER_SPACE_BASE "0x1000" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_USER_SPACE_SIZE "${CONFIG_MAX_VIRTUAL_ADDRESS} / 2 - ${CONFIG_USER_SPACE_BASE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_KERNEL_SPACE_BASE "${CONFIG_MAX_VIRTUAL_ADDRESS} / 2" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_KERNEL_SPACE_SIZE "${CONFIG_MAX_VIRTUAL_ADDRESS} - ${CONFIG_KERNEL_SPACE_BASE}" OUTPUT_FORMAT HEXADECIMAL)

    math(EXPR CONFIG_MAPPED_SPACE_BASE "${CONFIG_KERNEL_SPACE_BASE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_MAPPED_SPACE_SIZE "${CONFIG_MAX_PHYSICAL_ADDRESS}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_CAPABILITY_SPACE_BASE "${CONFIG_MAPPED_SPACE_BASE} + ${CONFIG_MAPPED_SPACE_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_CAPABILITY_SPACE_SIZE "512 * 512 * 0x1000" OUTPUT_FORMAT HEXADECIMAL) # 1GB
    math(EXPR CONFIG_TASK_SPACE_BASE "${CONFIG_CAPABILITY_SPACE_BASE} + ${CONFIG_CAPABILITY_SPACE_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_TASK_SPACE_SIZE "0x1000 * ${CONFIG_MAX_TASKS}" OUTPUT_FORMAT HEXADECIMAL)

    math(EXPR CONFIG_ACTUAL_VIRTUAL_SPACE_SIZE "${CONFIG_TASK_SPACE_BASE} + ${CONFIG_TASK_SPACE_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR CONFIG_ACTUAL_KERNEL_SPACE_SIZE "${CONFIG_ACTUAL_VIRTUAL_SPACE_SIZE} - ${CONFIG_KERNEL_SPACE_BASE}" OUTPUT_FORMAT HEXADECIMAL)

    if(${CONFIG_ACTUAL_VIRTUAL_SPACE_SIZE} GREATER_EQUAL ${CONFIG_MAX_VIRTUAL_ADDRESS})
      message(FATAL_ERROR "The upper limit of the virtual address space has been reached.")
    else()
      message(NOTICE "Actual virtual space size: ${CONFIG_ACTUAL_VIRTUAL_SPACE_SIZE}")
    endif()

    add_compile_definitions(
      CONFIG_MAX_RESERVED_REGIONS=${CONFIG_MAX_RESERVED_REGIONS}
      CONFIG_MAX_DEVICE_REGIONS=${CONFIG_MAX_DEVICE_REGIONS}
      CONFIG_MAX_MEMORY_REGIONS=${CONFIG_MAX_MEMORY_REGIONS}
      CONFIG_MAX_CORES=${CONFIG_MAX_CORES}
      CONFIG_MAX_TASKS=${CONFIG_MAX_TASKS}
      CONFIG_MAX_VIRTUAL_ADDRESS=${CONFIG_MAX_VIRTUAL_ADDRESS}
      CONFIG_MAX_PHYSICAL_ADDRESS=${CONFIG_MAX_PHYSICAL_ADDRESS}
      CONFIG_USER_SPACE_BASE=${CONFIG_USER_SPACE_BASE}
      CONFIG_USER_SPACE_SIZE=${CONFIG_USER_SPACE_SIZE}
      CONFIG_KERNEL_SPACE_BASE=${CONFIG_KERNEL_SPACE_BASE}
      CONFIG_KERNEL_SPACE_SIZE=${CONFIG_KERNEL_SPACE_SIZE}
      CONFIG_MAPPED_SPACE_BASE=${CONFIG_MAPPED_SPACE_BASE}
      CONFIG_MAPPED_SPACE_SIZE=${CONFIG_MAPPED_SPACE_SIZE}
      CONFIG_CAPABILITY_SPACE_BASE=${CONFIG_CAPABILITY_SPACE_BASE}
      CONFIG_CAPABILITY_SPACE_SIZE=${CONFIG_CAPABILITY_SPACE_SIZE}
      CONFIG_TASK_SPACE_BASE=${CONFIG_TASK_SPACE_BASE}
      CONFIG_TASK_SPACE_SIZE=${CONFIG_TASK_SPACE_SIZE}
    )
  else()
    message(FATAL_ERROR "Unsupported arch: ${arch}")
  endif()
endmacro(declare_arch)
