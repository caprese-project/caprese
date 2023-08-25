.extern __global_pointer$
.type __global_pointer$, @object
.extern _start
.type _start, @function
.extern __init_startup_routine
.type __init_startup_routine, @function
.extern exit
.type exit, @function

.section .text.entry

.global _entry
.type _entry, @function
_entry:
  mv s0, a0
  la sp, _stack
  call __init_startup_routine
  beqz a0, exit
  mv a3, a1
  mv a1, a0
  li a2, 0
  j _start

.section .data

.global _stack
.type _stack, @object
.p2align 12
.space 4096
_stack:

.section .apm, "a", @progbits
