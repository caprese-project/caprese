/**
 * @file linker.lds
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Linker script
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#include <init/generated/apm_size.h>

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = CONFIG_USER_PAYLOAD_BASE_ADDRESS;

  .text :
  {
    *(.text.entry)
    *(.text)
    *(.text.*)
  }

  . = ALIGN(16);

  .rodata :
  {
    *(.srodata*)
    . = ALIGN(16);
    *(.rodata)
    . = ALIGN(16);
    *(.rodata.*)
  }

  . = ALIGN(16);

  .data :
  {
    __global_pointer$ = . + 0x800;
    *(.sdata*)
    *(.data)
    *(.data.*)
    . = ALIGN(16);
    *(.sbss*)
    *(.bss)
    *(.bss.*)
    . = ALIGN(16);
    __init_array_start = .;
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)
           SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array .ctors))
    __init_array_end = .;
    __fini_array_start = .;
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)
           SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP(*(.fini_array .dtors))
    __fini_array_end = .;
  }

  . = ALIGN(16);

  _apm_start = .;

  .apm :
  {
    *(.apm)
    KEEP(*(.apm))
    . += CONFIG_APM_SIZE;
  }

  _apm_end = .;

  . = ALIGN(0x1000);

  _init_end = .;
}
