#ifndef NDEBUG

#include <caprese/util/printf.h>

#include <lib/syscall.h>

void _printd(const char* fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  for (char ch : "[DEBUG] ") {
    sys_debug_putchar(ch);
  }
  caprese::printf_template::printf(fmt, arg, sys_debug_putchar);
  va_end(arg);
}

#endif // NDEBUG
