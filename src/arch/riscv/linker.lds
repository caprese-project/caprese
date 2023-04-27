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
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x80000000;

  PROVIDE(begin_of_kernel = .);

  . = 0x80200000;
  .text :
  {
    *(.text.start)
    *(.text)
  }
  . = ALIGN(16);
  .rodata :
  {
    *(.srodata*)
    . = ALIGN(16);
    *(.rodata)
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
  . = ALIGN(16);
  .stack :
  {
    *(.stack)
  }

  . = ALIGN(16);
  PROVIDE(end_of_kernel = .);
}
