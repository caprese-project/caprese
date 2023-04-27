#include <caprese/arch/riscv/sbi.h>

namespace caprese::arch {
  void sbi_set_timer(uintptr_t stime) {
    asm volatile("mv a0, %0" : : "r"(stime));
    asm volatile("li a7, 0");
    asm volatile("ecall");
  }

  void sbi_putc(int ch) {
    asm volatile("mv a0, %0" : : "r"(ch));
    asm volatile("li a7, 1");
    asm volatile("ecall");
  }
} // namespace caprese::arch
