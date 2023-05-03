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
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

.extern __global_pointer$
.extern arch_main

.section .text.start

.global _start
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
  la s0, arch_main
  jr s0

spin:
  wfi
  j spin

.section .stack
.align 16
stack:
  .space 4096

.section .root_server
.incbin CONFIG_ROOT_SERVER_BIN
