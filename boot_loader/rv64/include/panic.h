/**
 * @file panic.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#ifndef BOOT_LOADER_PANIC_H_
#define BOOT_LOADER_PANIC_H_

#include <cstdarg>
#include <cstdio>

[[noreturn]] inline void panic(const char* fmt, ...) {
  printf("BOOT LOADER PANIC: ");

  va_list arg;
  va_start(arg, fmt);
  vprintf(fmt, arg);
  va_end(arg);

  printf("\n");

  while (true) {
    asm volatile("wfi");
  }
}

#endif // BOOT_LOADER_PANIC_H_
