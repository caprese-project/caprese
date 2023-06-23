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
.extern arch_main
.type arch_main, @function
.global _begin_of_root_server
.type _begin_of_root_server, @object
.global _end_of_root_server
.type _end_of_root_server, @object
.global _start
.type _start, @function

.section .text.entry

_start:
  # Without norelax option, "auipc gp, %pcrel_hi(__global_pointer$)" is optimized to "auipc gp, gp".
  .option push
  .option norelax
    1:
      auipc gp, %pcrel_hi(__global_pointer$)
      addi  gp, gp, %pcrel_lo(1b)
  .option pop

  la sp, stack
  # Boot Loader stores boot_info in a0.
  la t0, arch_main
  jr t0

spin:
  wfi
  j spin

.section .data
.balign 16
.space 4096
stack:

.section .data
.balign 4096
_begin_of_root_server:
.incbin CONFIG_ROOT_SERVER_BIN
_end_of_root_server:
