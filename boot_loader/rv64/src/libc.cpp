/**
 * @file libc.cpp
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
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <libcaprese/util/printf.h>

#include "panic.h"

namespace {
  int errno_value = 0;

  inline void sbi_console_putchar(int ch) {
    asm volatile("mv a7, %0" : : "r"(1));
    asm volatile("mv a6, %0" : : "r"(0));
    asm volatile("mv a0, %0" : : "r"(ch));
    asm volatile("ecall");
  }
} // namespace

extern "C" {
  int* __errno(void) {
    return &errno_value;
  }

  [[noreturn]] void __assert_func(const char* file, int line, const char* function, const char* condition) {
    printf("Assertion failed: %s, %s at %s:%d\n", condition, function, file, line);
    panic("ASSERTION FAILED");
  }

  uint32_t __bswapsi2(uint32_t value) {
    return ((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8) | ((value & 0x0000ff00) << 8) | ((value & 0x000000ff) << 24);
  }

  int strcmp(const char* str1, const char* str2) {
    while (*str1 != '\0' && *str1 == *str2) {
      ++str1;
      ++str2;
    }
    return *str1 - *str2;
  }

  int strncmp(const char* str1, const char* str2, size_t length) {
    int res = 0;
    while (length) {
      res = *str1 - *str2;
      ++str1;
      ++str2;
      if (res != 0 || *str1 == '\0' || *str2 == '\0') {
        break;
      }
      --length;
    }
    return res;
  }

  size_t strlen(const char* str) {
    size_t length = 0;
    while (str[length]) {
      ++length;
    }
    return length;
  }

  char* strchr(const char* str, int ch) {
    char* p = const_cast<char*>(str);
    while (true) {
      if (*p == ch) {
        return p;
      }
      if (*p == '\0') {
        return nullptr;
      }
      ++p;
    }
  }

  void* memset(void* src, int ch, size_t n) {
    unsigned char  c = ch;
    unsigned char* p = reinterpret_cast<unsigned char*>(src);
    while (n-- > 0) {
      *p++ = c;
    }
    return src;
  }

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
      sbi_console_putchar(c);
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
