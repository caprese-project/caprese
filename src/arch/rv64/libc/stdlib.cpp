#include <kernel/arch/sbi.h>

extern "C" {
  [[noreturn]] void _Exit([[maybe_unused]] int status) {
    while (true) {
      sbi_shutdown();
      asm volatile("wfi");
    }
  }
}
