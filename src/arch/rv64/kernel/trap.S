.section .text
.extern _user_trap
.type _user_trap, @function
.extern _kernel_trap
.type _kernel_trap, @function

/* void _return_to_user_mode(frame_t*) */
.global _return_to_user_mode
.type _return_to_user_mode, @function
.balign 4
_return_to_user_mode:
  csrw sscratch, a0

  ld t0, 248(a0) # frame.sepc
  ld t1, 256(a0) # frame.satp
  la t2, user_trap
  csrw sepc, t0
  csrw satp, t1
  csrw stvec, t2
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
  ld a1, 80(a0)
  ld a2, 88(a0)
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
  ld a0, 72(a0)

  sfence.vma zero, zero

  sret

.balign 4
user_trap:
  csrrw a0, sscratch, a0

  sd ra, 0(a0)
  sd sp, 8(a0)
  sd gp, 16(a0)
  sd tp, 24(a0)
  sd t0, 32(a0)
  sd t1, 40(a0)
  sd t2, 48(a0)
  sd s0, 56(a0)
  sd s1, 64(a0)
  sd a1, 80(a0)
  sd a2, 88(a0)
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
  ld sp, 272(a0)

  mv t0, a0
  csrr a0, sscratch
  sd a0, 72(t0)

  la t0, kernel_trap
  csrw stvec, t0

  j _user_trap

.balign 4
kernel_trap:
  addi sp, sp, -256

  sd ra, 0(sp)
  sd sp, 8(sp)
  sd gp, 16(sp)
  sd tp, 24(sp)
  sd t0, 32(sp)
  sd t1, 40(sp)
  sd t2, 48(sp)
  sd s0, 56(sp)
  sd s1, 64(sp)
  sd a0, 72(sp)
  sd a1, 80(sp)
  sd a2, 88(sp)
  sd a3, 96(sp)
  sd a4, 104(sp)
  sd a5, 112(sp)
  sd a6, 120(sp)
  sd a7, 128(sp)
  sd s2, 136(sp)
  sd s3, 144(sp)
  sd s4, 152(sp)
  sd s5, 160(sp)
  sd s6, 168(sp)
  sd s7, 176(sp)
  sd s8, 184(sp)
  sd s9, 192(sp)
  sd s10, 200(sp)
  sd s11, 208(sp)
  sd t3, 216(sp)
  sd t4, 224(sp)
  sd t5, 232(sp)
  sd t6, 240(sp)

  call _kernel_trap

  ld ra, 0(sp)
  ld sp, 8(sp)
  ld gp, 16(sp)
  ld t0, 32(sp)
  ld t1, 40(sp)
  ld t2, 48(sp)
  ld s0, 56(sp)
  ld s1, 64(sp)
  ld a0, 72(sp)
  ld a1, 80(sp)
  ld a2, 88(sp)
  ld a3, 96(sp)
  ld a4, 104(sp)
  ld a5, 112(sp)
  ld a6, 120(sp)
  ld a7, 128(sp)
  ld s2, 136(sp)
  ld s3, 144(sp)
  ld s4, 152(sp)
  ld s5, 160(sp)
  ld s6, 168(sp)
  ld s7, 176(sp)
  ld s8, 184(sp)
  ld s9, 192(sp)
  ld s10, 200(sp)
  ld s11, 208(sp)
  ld t3, 216(sp)
  ld t4, 224(sp)
  ld t5, 232(sp)
  ld t6, 240(sp)

  addi sp, sp, 256

  sret
