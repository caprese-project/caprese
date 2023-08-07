/**
 * @file panic.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare a function to safely stop the kernel in the event of a fatal condition.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#ifndef CAPRESE_UTIL_PANIC_H_
#define CAPRESE_UTIL_PANIC_H_

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <caprese/arch/system.h>

namespace caprese::inline util {
  [[noreturn]] inline void panic(const char* fmt, ...) {
    printf("KERNEL PANIC: ");

    va_list arg;
    va_start(arg, fmt);
    vprintf(fmt, arg);
    va_end(arg);

    printf("\n\n");

    arch::dump_system_context();

    printf("\n");

    abort();
  }
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_PANIC_H_
