/**
 * @file linker.ld
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Linker script
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese_sample_user_servers/blob/master/LICENSE
 */

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x1000;

  .text : {
    *(.text.start)
    *(.text)
    *(.text.*)
  }

  . = ALIGN(16);
  .rodata : {
    *(.srodata)
    . = ALIGN(16);
    *(.rodata)
    . = ALIGN(16);
    *(.rodata.*)
  }

  . = ALIGN(16);
  .data : {
    __global_pointer$ = . + 0x800;
    *(.sdata)
    *(.data)
    *(.data.*)
  }

  . = ALIGN(16);
  .bss : {
    *(.bss)
    *(.bss.*)
  }
}
