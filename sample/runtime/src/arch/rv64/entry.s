/**
 * @file entry.S
 * @author cosocaf (cosocaf@gmail.com)
 * @brief asm entry point
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

.extern __global_pointer$
.type __global_pointer$, @object
.extern __runtime_startup_routine
.type __runtime_startup_routine, @function
.extern main
.type main, @function
.extern exit
.type exit, @function

.section .text
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

  # a0: init task handle
  # a1: this task handle
  # a2: apm task handle
  # a3: heap start address
  # s0: argc
  # s1: argv

  call __runtime_startup_routine
  bnez a0, exit_phase
  mv a0, s0
  mv a1, s1
  call main

exit_phase:
  j exit
