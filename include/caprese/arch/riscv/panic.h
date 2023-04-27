#ifndef CAPRESE_ARCH_RISCV_PANIC_H_
#define CAPRESE_ARCH_RISCV_PANIC_H_

#ifdef CONFIG_ARCH_RISCV

namespace caprese::arch {
  void              dump_context();
  [[noreturn]] void halt();
} // namespace caprese::arch

#endif // CONFIG_ARCH_RISCV

#endif // CAPRESE_ARCH_RISCV_PANIC_H_
