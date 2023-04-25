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

#include <caprese/lib/console.h>
#include <caprese/kernel/panic.h>

#include <cstdarg>
#include <cstdint>
#include <cstddef>

namespace caprese {
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
        arch::console_put(buf[i]);
      }
    }

    void print_ptr(uint64_t ptr) {
      arch::console_put('0');
      arch::console_put('x');
      for (size_t i = 0; i < sizeof(uint64_t) * 2; i++, ptr <<= 4) {
        arch::console_put(digits[ptr >> (sizeof(uint64_t) * 8 - 4)]);
      }
    }

    void print_str(const char* str) {
      if (str == nullptr) {
        str = "null";
      }
      do {
        arch::console_put(*str);
      } while(*++str);
    }

    void vprintf(const char* fmt, va_list ap) {
      for(auto p = fmt; *p != '\0'; ++p) {
        if(*p != '%') [[likely]] {
          arch::console_put(*p);
          continue;
        }

        switch(*++p) {
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
            arch::console_put(va_arg(ap, int));
            break;
          case 'p':
            print_ptr(va_arg(ap, uint64_t));
            break;
          case 's':
            print_str(va_arg(ap, char*));
            break;
          case '%':
          case '\0':
            arch::console_put('%');
            break;
          default:
            arch::console_put('%');
            arch::console_put(*p);
            break;
        }
      }
    }
  }

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
    print("[WARN] %s: ", tag);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    print("\n");
  }
  void log_info(const char* tag, const char* fmt, ...) {
    print("[INFO] %s: ", tag);

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
}
