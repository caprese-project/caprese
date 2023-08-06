# Caprese
#
# (c) 2023 cosocaf
#
# This project is released under the MIT License.
# See https://github.com/cosocaf/caprese/blob/master/LICENSE

cmake_minimum_required(VERSION 3.16)

function(boot_template name)
  add_executable(${name})

  target_sources(
    ${name}
    PRIVATE
    entry.S
    start.cpp
  )

  target_compile_features(${name} PRIVATE cxx_std_23)
  target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CONFIG_CXX_OPTIONS}>)

  target_compile_definitions(${name} PRIVATE CONFIG_BOOT_PAYLOAD_BASE=${CONFIG_BOOT_PAYLOAD_BASE})

  target_include_directories(${name} PRIVATE ${ROOT_DIR}/include)

  target_link_libraries(${name} libc)

  target_link_options(
    ${name}
    PRIVATE
    -T ${CMAKE_CURRENT_BINARY_DIR}/${name}-linker.ld
    -nostdlib
    -z max-page-size=4096
  )
  set_target_properties(${name} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds)

  add_custom_target(
    ${name}_linker
    COMMAND ${CMAKE_C_COMPILER} -DCONFIG_BOOT_PAYLOAD_BASE=${CONFIG_BOOT_PAYLOAD_BASE} -I ${CMAKE_BINARY_DIR}/generated -E -P -x c ${CMAKE_CURRENT_SOURCE_DIR}/linker.lds >${CMAKE_CURRENT_BINARY_DIR}/${name}-linker.ld
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${name}-linker.ld
  )
  add_dependencies(${name} ${name}_linker)
endfunction(boot_template)

boot_template(prebuild_boot)

add_custom_target(
  kernel_payload_base
  COMMAND sh "-c" "${CMAKE_OBJDUMP} $<TARGET_FILE:prebuild_boot> -h | awk '/.payload/{print \"#define CONFIG_KERNEL_PAYLOAD_BASE 0x\"$4}' > ${GEN_DIR}/kernel_payload_base.h"
  BYPRODUCTS ${GEN_DIR}/kernel_payload_base.h
  VERBATIM
)
add_dependencies(kernel_payload_base prebuild_boot)

boot_template(boot)
add_dependencies(boot_linker kernel_payload kernel_payload_size)

add_custom_target(
  boot_payload
  COMMAND ${CMAKE_OBJCOPY} -O binary --update-section .payload=$<TARGET_FILE_DIR:kernel>/payload $<TARGET_FILE:boot> $<TARGET_FILE_DIR:boot>/payload
)
add_dependencies(boot_payload boot kernel_payload kernel_payload_size)
