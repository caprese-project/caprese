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

#include <apm/generated/console_size.h>
#include <apm/generated/shell_size.h>
#include <apm/generated/echo_size.h>
#include <apm/generated/pingpong_size.h>

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x800000;

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

  _console_start = .;

  .console :
  {
    *(.console)
    KEEP(*(.console))
    . += CONFIG_CONSOLE_SIZE;
  }

  . = ALIGN(16);

  _shell_start = .;

  .shell :
  {
    *(.shell)
    KEEP(*(.shell))
    . += CONFIG_SHELL_SIZE;
  }

  . = ALIGN(16);

  _echo_start = .;

  .echo :
  {
    *(.echo)
    KEEP(*(.echo))
    . += CONFIG_ECHO_SIZE;
  }

  _pingpong_start = .;

  .pingpong :
  {
    *(.pingpong)
    KEEP(*(.pingpong))
    . += CONFIG_PINGPONG_SIZE;
  }
}
