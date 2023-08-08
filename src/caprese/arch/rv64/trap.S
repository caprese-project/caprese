/**
 * @file head.S
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Defines functions related to traps.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

.section .data
.extern _core_local_storage_table
.type _core_local_storage_table, @object

.section .text
.extern _user_trap
.type _user_trap, @function

.global _jump_to_kernel_entry
.type _jump_to_kernel_entry, @function
.balign 4
_jump_to_kernel_entry:
  mv a0, s1
  jr s0

.global _return_to_user_mode
.type _return_to_user_mode, @function
.balign 4
_return_to_user_mode:
  ld t0, 248(a0)
  ld t1, 256(a0)
  la t2, user_trap
  csrw sepc, t0
  csrw satp, t1
  csrw stvec, t2
  csrw sscratch, tp
  sd tp, 264(a0)

  ld ra, 0(a0)
  ld sp, 8(a0)
  ld gp, 16(a0)
  ld tp, 24(a0)
  ld t0, 32(a0)
  ld t1, 40(a0)
  ld t2, 48(a0)
  ld s0, 56(a0)
  ld s1, 64(a0)
  ld a1, 72(a0)
  ld a2, 80(a0)
  ld a3, 96(a0)
  ld a4, 104(a0)
  ld a5, 112(a0)
  ld a6, 120(a0)
  ld a7, 128(a0)
  ld s2, 136(a0)
  ld s3, 144(a0)
  ld s4, 152(a0)
  ld s5, 160(a0)
  ld s6, 168(a0)
  ld s7, 176(a0)
  ld s8, 184(a0)
  ld s9, 192(a0)
  ld s10, 200(a0)
  ld s11, 208(a0)
  ld t3, 216(a0)
  ld t4, 224(a0)
  ld t5, 232(a0)
  ld t6, 240(a0)
  ld a0, 248(a0)

  sret

.balign 4
user_trap:
  csrrw tp, sscratch, tp
  csrrw a0, stvec, a0

  la a0, _core_local_storage_table
  slli tp, tp, CONFIG_CLS_SIZE_BIT
  add a0, a0, tp
  ld a0, 8(a0) # current tid
  srliw a0, a0, (32 - CONFIG_TASK_SIZE_BIT)
  slliw a0, a0, (32 - CONFIG_TASK_SIZE_BIT)
  slli a0, a0, CONFIG_TASK_SIZE_BIT
  li tp, CONFIG_TASK_SPACE_BASE
  add tp, tp, a0 # current task
  li a0, (CONFIG_ARCH_TASK_OFFSET + CONFIG_ARCH_TASK_TRAP_FRAME_OFFSET)
  add a0, tp, a0 # trap frame

  csrrw tp, sscratch, tp

  sd ra, 0(a0)
  sd sp, 8(a0)
  sd gp, 16(a0)
  sd tp, 24(a0)
  sd t0, 32(a0)
  sd t1, 40(a0)
  sd t2, 48(a0)
  sd s0, 56(a0)
  sd s1, 64(a0)
  sd a1, 72(a0)
  sd a2, 80(a0)
  sd a3, 96(a0)
  sd a4, 104(a0)
  sd a5, 112(a0)
  sd a6, 120(a0)
  sd a7, 128(a0)
  sd s2, 136(a0)
  sd s3, 144(a0)
  sd s4, 152(a0)
  sd s5, 160(a0)
  sd s6, 168(a0)
  sd s7, 176(a0)
  sd s8, 184(a0)
  sd s9, 192(a0)
  sd s10, 200(a0)
  sd s11, 208(a0)
  sd t3, 216(a0)
  sd t4, 224(a0)
  sd t5, 232(a0)
  sd t6, 240(a0)

  csrr t0, sepc
  csrr t1, satp
  sd t0, 248(a0)
  sd t1, 256(a0)

  ld tp, 264(a0)

  mv t0, a0
  csrr a0, stvec
  sd a0, 248(t0)

  la t0, _core_local_storage_table
  slli t1, t1, CONFIG_CLS_SIZE_BIT
  add t0, t0, t1
  ld t0, 8(t0) # current tid
  srliw t0, t0, (32 - CONFIG_TASK_SIZE_BIT)
  slliw t0, t0, (32 - CONFIG_TASK_SIZE_BIT)
  addi t0, t0, 1
  slli t0, t0, 12
  li t1, CONFIG_STACK_SPACE_BASE
  add sp, t0, t1

  la t0, kernel_trap
  csrw stvec, t0

  la t0, _user_trap
  jr t0

.balign 4
kernel_trap: #TODO: impl
  wfi
  j kernel_trap

.global _switch_context
.type _switch_context, @function
.global _load_context
.type _load_context, @function

.balign 4
_switch_context:
  sd ra, 0(a0)
  sd sp, 8(a0)
  sd s0, 16(a0)
  sd s1, 24(a0)
  sd s2, 32(a0)
  sd s3, 40(a0)
  sd s4, 48(a0)
  sd s5, 56(a0)
  sd s6, 64(a0)
  sd s7, 72(a0)
  sd s8, 80(a0)
  sd s9, 88(a0)
  sd s10, 96(a0)
  sd s11, 104(a0)

.balign 4
_load_context:
  ld ra, 0(a1)
  ld sp, 8(a1)
  ld s0, 16(a1)
  ld s1, 24(a1)
  ld s2, 32(a1)
  ld s3, 40(a1)
  ld s4, 48(a1)
  ld s5, 56(a1)
  ld s6, 64(a1)
  ld s7, 72(a1)
  ld s8, 80(a1)
  ld s9, 88(a1)
  ld s10, 96(a1)
  ld s11, 104(a1)

  ret

# .global kernel_vector
# .type kernel_vector, @function
# .extern kernel_trap
# .type kernel_trap, @function
# .balign 4
# kernel_vector:
#   addi sp, sp, -256

#   sd ra, 0(sp)
#   sd sp, 8(sp)
#   sd gp, 16(sp)
#   sd tp, 24(sp)
#   sd t0, 32(sp)
#   sd t1, 40(sp)
#   sd t2, 48(sp)
#   sd s0, 56(sp)
#   sd s1, 64(sp)
#   sd a0, 72(sp)
#   sd a1, 80(sp)
#   sd a2, 88(sp)
#   sd a3, 96(sp)
#   sd a4, 104(sp)
#   sd a5, 112(sp)
#   sd a6, 120(sp)
#   sd a7, 128(sp)
#   sd s2, 136(sp)
#   sd s3, 144(sp)
#   sd s4, 152(sp)
#   sd s5, 160(sp)
#   sd s6, 168(sp)
#   sd s7, 176(sp)
#   sd s8, 184(sp)
#   sd s9, 192(sp)
#   sd s10, 200(sp)
#   sd s11, 208(sp)
#   sd t3, 216(sp)
#   sd t4, 224(sp)
#   sd t5, 232(sp)
#   sd t6, 240(sp)

#   call kernel_trap

#   ld ra, 0(sp)
#   ld sp, 8(sp)
#   ld gp, 16(sp)
#   ld t0, 32(sp)
#   ld t1, 40(sp)
#   ld t2, 48(sp)
#   ld s0, 56(sp)
#   ld s1, 64(sp)
#   ld a0, 72(sp)
#   ld a1, 80(sp)
#   ld a2, 88(sp)
#   ld a3, 96(sp)
#   ld a4, 104(sp)
#   ld a5, 112(sp)
#   ld a6, 120(sp)
#   ld a7, 128(sp)
#   ld s2, 136(sp)
#   ld s3, 144(sp)
#   ld s4, 152(sp)
#   ld s5, 160(sp)
#   ld s6, 168(sp)
#   ld s7, 176(sp)
#   ld s8, 184(sp)
#   ld s9, 192(sp)
#   ld s10, 200(sp)
#   ld s11, 208(sp)
#   ld t3, 216(sp)
#   ld t4, 224(sp)
#   ld t5, 232(sp)
#   ld t6, 240(sp)

#   addi sp, sp, 256

#   sret

# .section .text.trampoline
# .global begin_of_trampoline
# .global end_of_trampoline
# # void trampoline_user_vector();
# .global trampoline_user_vector
# .type trampoline_user_vector, @function
# # void trampoline_return_to_user_mode(uintptr_t trapframe, uintptr_t satp);
# .global trampoline_return_to_user_mode
# .type trampoline_return_to_user_mode, @function

# begin_of_trampoline:

# trampoline_user_vector:
#   csrw sscratch, a0

#   auipc a0, 0
#   srli a0, a0, 12
#   slli a0, a0, 12
#   addi a0, a0, 16

#   sd ra, 40(a0)
#   sd sp, 48(a0)
#   sd gp, 56(a0)
#   sd tp, 64(a0)
#   sd t0, 72(a0)
#   sd t1, 80(a0)
#   sd t2, 88(a0)
#   sd s0, 96(a0)
#   sd s1, 104(a0)
#   sd a1, 120(a0)
#   sd a2, 128(a0)
#   sd a3, 136(a0)
#   sd a4, 144(a0)
#   sd a5, 152(a0)
#   sd a6, 160(a0)
#   sd a7, 168(a0)
#   sd s2, 176(a0)
#   sd s3, 184(a0)
#   sd s4, 192(a0)
#   sd s5, 200(a0)
#   sd s6, 208(a0)
#   sd s7, 216(a0)
#   sd s8, 224(a0)
#   sd s9, 232(a0)
#   sd s10, 240(a0)
#   sd s11, 248(a0)
#   sd t3, 256(a0)
#   sd t4, 264(a0)
#   sd t5, 272(a0)
#   sd t6, 280(a0)

#   csrr t0, sscratch
#   sd t0, 112(a0)

#   // kernel_sp
#   ld sp, 8(a0)
#   // kernel_hartid
#   ld tp, 32(a0)
#   // kernel_trap
#   ld t0, 16(a0)
#   // kernel_satp
#   ld t1, 0(a0)

#   # sfence.vma zero, zero
#   # csrw satp, t1
#   # sfence.vma zero, zero

#   jr t0
