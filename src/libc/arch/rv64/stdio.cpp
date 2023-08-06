/**
 * @file stdio.cpp
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

#include <cstdarg>
#include <cstdio>

#include <caprese/arch/rv64/sbi.h>

#include <libcaprese/util/printf.h>

extern "C" {
  int printf(const char* fmt, ...) {
    va_list arg;
    int     len;
    va_start(arg, fmt);
    len = vprintf(fmt, arg);
    va_end(arg);
    return len;
  }

  int vprintf(const char* fmt, va_list arg) {
    size_t     size     = 0;
    const auto callback = [&size](char c) {
      caprese::arch::sbi_console_putchar(c);
      ++size;
    };
    libcaprese::util::printf_template::printf(fmt, arg, callback);
    return size;
  }

  int snprintf(char* dst, size_t n, const char* fmt, ...) {
    va_list arg;
    int     len;
    va_start(arg, fmt);
    len = vsnprintf(dst, n, fmt, arg);
    va_end(arg);
    return len;
  }

  int vsnprintf(char* dst, size_t n, const char* fmt, va_list arg) {
    size_t     size     = 0;
    const auto callback = [&size, dst, n](char c) {
      if (size >= n - 1) {
        return;
      }
      dst[size] = c;
      ++size;
    };
    libcaprese::util::printf_template::printf(fmt, arg, callback);
    dst[size] = '\0';
    return size;
  }
}
