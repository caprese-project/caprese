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
 * @see https://github.com/cosocaf/caprese_sample_user_servers/blob/master/LICENSE
 */

.extern __global_pointer$
.type __global_pointer$, @object
.extern main
.type main, @function

.section .text.start

.global _start
.type _start, @function
_start:
  .option push
  .option norelax
    1:
      auipc gp, %pcrel_hi(__global_pointer$)
      addi  gp, gp, %pcrel_lo(1b)
  .option pop

  la sp, stack
  la s0, main
  jr s0

spin:
  wfi
  j spin

.section .data
.balign 16
.space 4096
stack:
