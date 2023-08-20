#ifndef NDEBUG

#include <caprese/util/printf.h>

#include <lib/syscall.h>

void _vprintf(const char* fmt, va_list arg) {
  caprese::printf_template::printf(fmt, arg, sys_debug_putchar);
}

void _printf(const char* fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  _vprintf(fmt, arg);
  va_end(arg);
}

void _printd(const char* fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  _printf("[DEBUG] [TID=0x%lx] ", sys_task_tid().result);
  caprese::printf_template::printf(fmt, arg, sys_debug_putchar);
  va_end(arg);
}

#endif // NDEBUG
