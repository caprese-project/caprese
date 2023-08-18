#ifndef NDEBUG

#include <syscall.h>

#include <caprese/util/printf.h>

void _printd(const char* fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  caprese::printf_template::printf(fmt, arg, sys_debug_putchar);
  va_end(arg);
}

#endif // NDEBUG
