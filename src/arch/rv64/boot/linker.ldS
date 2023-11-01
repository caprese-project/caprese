#if __has_include(<kernel_payload_size.h>)
#include <kernel_payload_size.h>
#endif

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = CONFIG_BOOT_PAYLOAD_BASE;

  _boot_start = .;

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
  }

  . = ALIGN(16);

  .bss :
  {
    *(.sbss*)
    *(.bss)
    *(.bss.*)
  }

  _boot_end = .;

  . = ALIGN(0x1000);

  _payload_start = .;

  .payload :
  {
    KEEP(*(.payload))
#ifdef CONFIG_KERNEL_PAYLOAD_SIZE
    . += CONFIG_KERNEL_PAYLOAD_SIZE;
#endif
  }

  _payload_end = .;

  . = ALIGN(16);

  .init :
  {
    *(.init)
    *(.init.*)
  }
}
