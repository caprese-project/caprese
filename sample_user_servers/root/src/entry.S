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
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

.extern main

.section .text.start

.global _start
_start:
  la sp, stack
  la s0, main
  jr s0

spin:
  wfi
  j spin

.section .data
.align 16
stack:
  .space 4096
