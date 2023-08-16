#include <syscall.h>

extern "C" [[noreturn]] void main() {
  for(char ch : "Hello\n") {
    sys_debug_putchar(ch);
  }
  while(true) {}
}
