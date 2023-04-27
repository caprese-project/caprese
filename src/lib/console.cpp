/**
 * @file console.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement a console output utility for the kernel.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>

#include <caprese/arch/riscv/sbi.h>
#include <caprese/kernel/panic.h>
#include <caprese/lib/console.h>

namespace caprese {
  namespace {
    inline void putc(int ch) {
#if defined(CONFIG_ARCH_RISCV)
      arch::sbi_putc(ch);
#endif // defined(CONFIG_ARCH_RISCV)
    }
  } // namespace

  namespace {
    constexpr auto digits = "0123456789ABCDEF";

    void print_int(int n, int base, bool sign) {
      char buf[16];
      if (sign && n < 0) {
        sign = true;
        n    = -n;
      } else {
        sign = false;
      }

      int i = 0;
      do {
        buf[i++] = digits[n % base];
      } while (n /= base);

      if (sign) {
        buf[i++] = '-';
      }

      while (--i >= 0) {
        putc(buf[i]);
      }
    }

    void print_ptr(uint64_t ptr) {
      putc('0');
      putc('x');
      for (size_t i = 0; i < sizeof(uint64_t) * 2; i++, ptr <<= 4) {
        putc(digits[ptr >> (sizeof(uint64_t) * 8 - 4)]);
      }
    }

    void print_str(const char* str) {
      if (str == nullptr) {
        str = "null";
      }
      do {
        putc(*str);
      } while (*++str);
    }

    void vprintf(const char* fmt, va_list ap) {
      for (auto p = fmt; *p != '\0'; ++p) {
        if (*p != '%') [[likely]] {
          putc(*p);
          continue;
        }

        switch (*++p) {
          case 'd':
            print_int(va_arg(ap, int32_t), 10, true);
            break;
          case 'u':
            print_int(va_arg(ap, uint32_t), 10, false);
            break;
          case 'x':
            print_int(va_arg(ap, int32_t), 16, true);
            break;
          case 'c':
            putc(va_arg(ap, int));
            break;
          case 'p':
            print_ptr(va_arg(ap, uint64_t));
            break;
          case 's':
            print_str(va_arg(ap, char*));
            break;
          case '%':
          case '\0':
            putc('%');
            break;
          default:
            putc('%');
            putc(*p);
            break;
        }
      }
    }
  } // namespace

  void print(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }

  void println(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }

  [[noreturn]] void log_fatal(const char* tag, const char* fmt, ...) {
    print("[FATAL] %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");

    panic("A fatal error has occurred.");
  }

  void log_error(const char* tag, const char* fmt, ...) {
    print("[ERROR] %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }

  void log_warn(const char* tag, const char* fmt, ...) {
    print("[WARN]  %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }

  void log_info(const char* tag, const char* fmt, ...) {
    print("[INFO]  %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }

  void log_debug(const char* tag, const char* fmt, ...) {
    print("[DEBUG] %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }

  void log_assert(const char* tag, bool condition, const char* fmt, ...) {
#ifndef NDEBUG
    if (!condition) {
      print("[ASSERT] %s: ", tag);

      va_list ap;
      va_start(ap, fmt);
      vprintf(fmt, ap);
      va_end(ap);

      panic("Assertion failed.");
    }
#endif
  }
} // namespace caprese
