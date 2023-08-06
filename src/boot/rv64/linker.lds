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

#if __has_include(<caprese/generated/kernel_payload_size.h>)
#include <caprese/generated/kernel_payload_size.h>
#endif
#define PAGE_SIZE 4096

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = CONFIG_BOOT_PAYLOAD_BASE;

  _boot_loader_start = .;

  .text :
  {
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
  }

  . = ALIGN(16);

  .bss :
  {
    *(.sbss*)
    *(.bss)
    *(.bss.*)
  }

  _boot_loader_end = .;

  . = ALIGN(PAGE_SIZE);

  .payload : {
    _payload_base = .;
    KEEP(*(.payload))
#ifdef CONFIG_KERNEL_PAYLOAD_SIZE
    . = . + CONFIG_KERNEL_PAYLOAD_SIZE;
#endif
  }

  . = ALIGN(PAGE_SIZE);

  .page_table :
  {
    _page_table = .;
    KEEP(*(.page_table))
  }
}
