#include <libcaprese/syscall.h>

extern "C" [[noreturn]] int main() {
  for (auto ch : "Hello, world!\n") {
    sys_debug_putchar(ch);
  }

  while (true) {
    asm volatile("wfi");
  }
}
