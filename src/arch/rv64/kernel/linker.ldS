#include <kernel_payload_base.h>
#include <root_task_payload_size.h>

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = CONFIG_MAPPED_SPACE_BASE + CONFIG_KERNEL_PAYLOAD_BASE;

  _kernel_start = .;

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

  . = ALIGN(0x1000);

  _kernel_end = .;

  .payload :
  {
    _payload_start = .;
    *(.payload)
    KEEP(*(.payload))
    . += CONFIG_ROOT_TASK_PAYLOAD_SIZE;
    . = ALIGN(0x1000);
    _payload_end = .;
    _root_task_stack_start = .;
    /* TODO: Make the stack size configurable. */
    . += 0x1000;
    _root_task_stack_end = .;
  }

  . = ALIGN(16);

  .init :
  {
    *(.init.*)
  }
}

