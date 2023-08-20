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

#include <caprese/generated/mm_size.h>
#include <caprese/generated/plic_size.h>
#include <caprese/generated/fs_size.h>

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = CONFIG_USER_PAYLOAD_BASE_ADDRESS;

  .text :
  {
    KEEP(*(.text.entry))
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
  }

  . = ALIGN(16);

  _mm_start = .;

  .mm :
  {
    *(.mm)
    KEEP(*(.mm))
    . += CONFIG_MM_SIZE;
  }

  _mm_end = .;

  . = ALIGN(16);

  _plic_start = .;

  .plic :
  {
    *(.plic)
    KEEP(*(.plic))
    . += CONFIG_PLIC_SIZE;
  }

  _plic_end = .;

  . = ALIGN(16);

  _fs_start = .;

  .fs :
  {
    *(.fs)
    KEEP(*(.fs))
    . += CONFIG_FS_SIZE;
  }

  _fs_end = .;

  . = ALIGN(0x1000);

  _init_end = .;
}
