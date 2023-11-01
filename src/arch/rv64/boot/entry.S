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

  la sp, stack
  # OpenSBI stores hartid in a0 and pointer to dtb in a1.
  j start

.section .init.data
.balign 16
.space 4096
stack:

.section .payload, "ax", @progbits
