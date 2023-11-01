#ifndef ARCH_RV64_KERNEL_ARCH_TASK_H_
#define ARCH_RV64_KERNEL_ARCH_TASK_H_

#include <cstddef>
#include <cstdint>

#include <kernel/page.h>

struct context_t {
  uintptr_t ra;
  uintptr_t sp;
  uintptr_t s0;
  uintptr_t s1;
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
};

struct trap_frame_t {
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

struct arch_task_t {
  context_t    context;
  trap_frame_t trap_frame;
  alignas(16) char kernel_stack[PAGE_SIZE];
};

constexpr uintptr_t LAST_REGISTER = offsetof(trap_frame_t, sepc) / sizeof(uintptr_t);

#endif // ARCH_RV64_KERNEL_ARCH_TASK_H_
