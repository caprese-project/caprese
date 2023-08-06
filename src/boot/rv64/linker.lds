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

#define PAGE_SIZE 4096
#define BOOT_LOADER_START 0x80200000

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = BOOT_LOADER_START;

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

  .kernel :
  {
    *(.kernel)
  }
}
