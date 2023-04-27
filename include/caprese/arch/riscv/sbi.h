#ifndef CAPRESE_ARCH_RISCV_SBI_H_
#define CAPRESE_ARCH_RISCV_SBI_H_

#ifdef CONFIG_ARCH_RISCV

#include <cstdint>

namespace caprese::arch {
  void sbi_set_timer(uintptr_t stime);
  void sbi_putc(int ch);
} // namespace caprese::arch

#endif // CONFIG_ARCH_RISCV

#endif // CAPRESE_ARCH_RISCV_SBI_H_
