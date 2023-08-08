/**
 * @file head.S
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
  # Boot loader stores hartid in a0 and pointer to dtb in a1.
  la t0, start
  jr t0

spin:
  wfi
  j spin

.section .data

.global _stack
.type _stack, @object
.p2align 12
.space 4096
_stack:

.section .payload, "ax", %progbits
.incbin	CONFIG_USER_PAYLOAD
