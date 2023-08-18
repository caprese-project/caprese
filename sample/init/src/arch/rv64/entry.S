.extern __global_pointer$
.type __global_pointer$, @object
.extern start
.type start, @function

.section .text.entry

.global _start
.type _start, @function
_start:
  # Without norelax option, "auipc gp, %pcrel_hi(__global_pointer$)" is optimized to "auipc gp, gp".
  .option push
  .option norelax
    1:
      auipc gp, %pcrel_hi(__global_pointer$)
      addi  gp, gp, %pcrel_lo(1b)
  .option pop

  la sp, _stack
  call start

spin:
  wfi
  j spin

.section .data

.global _stack
.type _stack, @object
.p2align 12
.space 4096
_stack:
