#include <caprese/task/syscall.h>

void putchar(char c) {
  asm volatile("li a7, %0" : : "i"(SYS_DEBUG_PUTCHAR));
  asm volatile("mv a0, %0" : : "r"(c));
  asm volatile("ecall");
}

extern "C" [[noreturn]] int main() {
  for (auto ch : "Hello, world!\n") {
    putchar(ch);
  }

  while (true) {
    asm volatile("wfi");
  }
}
