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
  . = CONFIG_ROOT_SERVER_BASE_ADDRESS;

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
