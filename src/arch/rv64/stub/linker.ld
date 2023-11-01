OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0xa0000000;

  .text :
  {
    *(.text)
  }
}
