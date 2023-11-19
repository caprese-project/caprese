#ifndef ARCH_RV64_KERNEL_CONTEXT_H_
#define ARCH_RV64_KERNEL_CONTEXT_H_

#include <cstdint>

#include <kernel/address.h>

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

void              switch_context(map_ptr<context_t> new_context, map_ptr<context_t> old_context);
[[noreturn]] void load_context(map_ptr<context_t> context);

#endif // ARCH_RV64_KERNEL_CONTEXT_H_
