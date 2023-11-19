#ifndef ARCH_RV64_KERNEL_FRAME_H_
#define ARCH_RV64_KERNEL_FRAME_H_

#include <cstddef>
#include <cstdint>

#include <libcaprese/syscall.h>

struct frame_t {
  uintptr_t ra;
  uintptr_t sp;
  uintptr_t gp;
  uintptr_t tp;
  uintptr_t t0;
  uintptr_t t1;
  uintptr_t t2;
  uintptr_t s0;
  uintptr_t s1;
  uintptr_t a0;
  uintptr_t a1;
  uintptr_t a2;
  uintptr_t a3;
  uintptr_t a4;
  uintptr_t a5;
  uintptr_t a6;
  uintptr_t a7;
  uintptr_t s2;
  uintptr_t s3;
  uintptr_t s4;
  uintptr_t s5;
  uintptr_t s6;
  uintptr_t s7;
  uintptr_t s8;
  uintptr_t s9;
  uintptr_t s10;
  uintptr_t s11;
  uintptr_t t3;
  uintptr_t t4;
  uintptr_t t5;
  uintptr_t t6;
  uintptr_t sepc;
  uintptr_t satp;
  uintptr_t hartid;
  uintptr_t stack;
};

static_assert(ARCH_REG_RA == offsetof(frame_t, ra) / sizeof(uintptr_t));
static_assert(ARCH_REG_SP == offsetof(frame_t, sp) / sizeof(uintptr_t));
static_assert(ARCH_REG_GP == offsetof(frame_t, gp) / sizeof(uintptr_t));
static_assert(ARCH_REG_TP == offsetof(frame_t, tp) / sizeof(uintptr_t));
static_assert(ARCH_REG_T0 == offsetof(frame_t, t0) / sizeof(uintptr_t));
static_assert(ARCH_REG_T1 == offsetof(frame_t, t1) / sizeof(uintptr_t));
static_assert(ARCH_REG_T2 == offsetof(frame_t, t2) / sizeof(uintptr_t));
static_assert(ARCH_REG_S0 == offsetof(frame_t, s0) / sizeof(uintptr_t));
static_assert(ARCH_REG_S1 == offsetof(frame_t, s1) / sizeof(uintptr_t));
static_assert(ARCH_REG_A0 == offsetof(frame_t, a0) / sizeof(uintptr_t));
static_assert(ARCH_REG_A1 == offsetof(frame_t, a1) / sizeof(uintptr_t));
static_assert(ARCH_REG_A2 == offsetof(frame_t, a2) / sizeof(uintptr_t));
static_assert(ARCH_REG_A3 == offsetof(frame_t, a3) / sizeof(uintptr_t));
static_assert(ARCH_REG_A4 == offsetof(frame_t, a4) / sizeof(uintptr_t));
static_assert(ARCH_REG_A5 == offsetof(frame_t, a5) / sizeof(uintptr_t));
static_assert(ARCH_REG_A6 == offsetof(frame_t, a6) / sizeof(uintptr_t));
static_assert(ARCH_REG_A7 == offsetof(frame_t, a7) / sizeof(uintptr_t));
static_assert(ARCH_REG_S2 == offsetof(frame_t, s2) / sizeof(uintptr_t));
static_assert(ARCH_REG_S3 == offsetof(frame_t, s3) / sizeof(uintptr_t));
static_assert(ARCH_REG_S4 == offsetof(frame_t, s4) / sizeof(uintptr_t));
static_assert(ARCH_REG_S5 == offsetof(frame_t, s5) / sizeof(uintptr_t));
static_assert(ARCH_REG_S6 == offsetof(frame_t, s6) / sizeof(uintptr_t));
static_assert(ARCH_REG_S7 == offsetof(frame_t, s7) / sizeof(uintptr_t));
static_assert(ARCH_REG_S8 == offsetof(frame_t, s8) / sizeof(uintptr_t));
static_assert(ARCH_REG_S9 == offsetof(frame_t, s9) / sizeof(uintptr_t));
static_assert(ARCH_REG_S10 == offsetof(frame_t, s10) / sizeof(uintptr_t));
static_assert(ARCH_REG_S11 == offsetof(frame_t, s11) / sizeof(uintptr_t));
static_assert(ARCH_REG_T3 == offsetof(frame_t, t3) / sizeof(uintptr_t));
static_assert(ARCH_REG_T4 == offsetof(frame_t, t4) / sizeof(uintptr_t));
static_assert(ARCH_REG_T5 == offsetof(frame_t, t5) / sizeof(uintptr_t));
static_assert(ARCH_REG_T6 == offsetof(frame_t, t6) / sizeof(uintptr_t));
static_assert(ARCH_REG_SEPC == offsetof(frame_t, sepc) / sizeof(uintptr_t));

uintptr_t               set_register(frame_t* frame, uintptr_t reg, uintptr_t value);
[[nodiscard]] uintptr_t get_register(frame_t* frame, uintptr_t reg);

#endif // ARCH_RV64_KERNEL_FRAME_H_
