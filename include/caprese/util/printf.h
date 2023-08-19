#ifndef CAPRESE_UTIL_PRINTF_H_
#define CAPRESE_UTIL_PRINTF_H_

#include <algorithm>
#include <cstdarg>
#include <cstring>

namespace caprese::inline util {
  class printf_template {
    static constexpr auto upper_digits = "0123456789ABCDEF";
    static constexpr auto lower_digits = "0123456789abcdef";

  private:
    template<typename PutFn>
    static void print_str(bool left_align, int width, int precision, const char* str, const PutFn& callback) {
      if (str == nullptr) {
        str = "null";
      }

      int len       = strlen(str);
      int max_width = 0;
      if (precision == 0) {
        max_width = len;
      } else {
        max_width = std::min(len, precision);
      }

      if (max_width < width && !left_align) {
        for (int i = 0; i < width - max_width; ++i) {
          callback(' ');
        }
      }

      for (int i = 0; i < max_width; ++i) {
        callback(str[i]);
      }

      if (max_width < width && left_align) {
        for (int i = 0; i < width - max_width; ++i) {
          callback(' ');
        }
      }
    }

    template<typename T, typename PutFn>
    static void print_int(bool left_align, bool sign, bool space, bool zero, bool apostrophe, bool hash, int width, int precision, T value, int base, bool upper_char, const PutFn& callback) {
      bool minus = false;

      if (value < static_cast<T>(0)) {
        value = -value;
        minus = true;
      }

      char buf[32];
      int  i      = 0;
      auto digits = upper_char ? upper_digits : lower_digits;
      do {
        buf[i++] = digits[value % base];
        if (apostrophe && i % 4 == 0) {
          buf[i++] = '\'';
        }
      } while (value /= base);

      width = std::max(width, precision);

      if (width && zero) {
        while (i < width) {
          buf[i++] = '0';
          if (apostrophe && i % 4 == 0 && i + 1 < width) {
            buf[i++] = '\'';
          }
        }
      }

      if (hash && base == 16) {
        buf[i++] = 'x';
        buf[i++] = '0';
      }

      if (minus) {
        buf[i++] = '-';
      } else if (sign) {
        buf[i++] = '+';
      } else if (space) {
        buf[i++] = ' ';
      }

      if (width && !zero && !left_align) {
        while (i < width) {
          buf[i++] = ' ';
        }
      }

      while (--i >= 0) {
        callback(buf[i]);
      }

      if (width && !zero && left_align) {
        while (i < width) {
          ++i;
          callback(' ');
        }
      }
    }

  public:
    template<typename PutFn>
    static void printf(const char* fmt, va_list arg, const PutFn& callback) {
      for (auto p = fmt; *p != '\0'; ++p) {
        if (*p != '%') [[likely]] {
          callback(*p);
          continue;
        }

        // Flags field

        bool minus      = false;
        bool plus       = false;
        bool space      = false;
        bool zero       = false;
        bool apostrophe = false;
        bool hash       = false;

        while (*++p) {
          switch (*p) {
            case '-':
              minus = true;
              break;
            case '+':
              plus = true;
              break;
            case ' ':
              space = true;
              break;
            case '0':
              zero = true;
              break;
            case '\'':
              apostrophe = true;
              break;
            case '#':
              hash = true;
              break;
            default:
              --p;
              goto flags_end;
          }
        }

      flags_end:

        // Width field

        int width = 0;

        ++p;
        if (*p == '*') {
          width = -1;
        } else {
          do {
            if (*p >= '0' && *p <= '9') {
              width *= 10;
              width += *p - '0';
            } else {
              --p;
              break;
            }
          } while (*++p);
        }

        // Precision field

        int precision = 0;
        ++p;
        if (*p == '.') {
          ++p;
          if (*p == '*') {
            precision = -1;
          } else {
            do {
              precision *= 10;
              if (*p >= '0' && *p <= '9') {
                precision += *p - '0';
              } else {
                --p;
                break;
              }
            } while (*++p);
          }
        } else {
          --p;
        }

        // Length field

        bool hh = false;
        bool h  = false;
        bool l  = false;
        bool ll = false;
        bool j  = false;
        bool z  = false;
        bool t  = false;

        switch (*++p) {
          case 'h':
            if (*++p == 'h') {
              hh = true;
            } else {
              --p;
              h = true;
            }
            break;
          case 'l':
            if (*++p == 'l') {
              ll = true;
            } else {
              --p;
              l = true;
            }
            break;
          case 'j':
            j = true;
            break;
          case 'z':
            z = true;
            break;
          case 't':
            t = true;
            break;
          default:
            --p;
            break;
        }

        // Type field

        switch (*++p) {
          case 'd':
          case 'i': {
            if (width == -1) {
              width = va_arg(arg, int);
            }
            if (precision == -1) {
              precision = va_arg(arg, int);
            }

            intmax_t value;
            if (hh) {
              value = static_cast<char>(va_arg(arg, int));
            } else if (h) {
              value = static_cast<short>(va_arg(arg, int));
            } else if (l) {
              value = va_arg(arg, long);
            } else if (ll) {
              value = va_arg(arg, long long);
            } else if (z) {
              value = va_arg(arg, size_t);
            } else if (j) {
              value = va_arg(arg, intmax_t);
            } else if (t) {
              value = va_arg(arg, ptrdiff_t);
            } else {
              value = va_arg(arg, int);
            }
            print_int(minus, plus, space, zero, apostrophe, hash, width, precision, value, 10, false, callback);
            break;
          }
          case 'u': {
            if (width == -1) {
              width = va_arg(arg, int);
            }
            if (precision == -1) {
              precision = va_arg(arg, int);
            }

            uintmax_t value;
            if (hh) {
              value = static_cast<unsigned char>(va_arg(arg, unsigned int));
            } else if (h) {
              value = static_cast<unsigned short>(va_arg(arg, unsigned int));
            } else if (l) {
              value = va_arg(arg, unsigned long);
            } else if (ll) {
              value = va_arg(arg, unsigned long long);
            } else if (z) {
              value = va_arg(arg, size_t);
            } else if (j) {
              value = va_arg(arg, uintmax_t);
            } else if (t) {
              value = va_arg(arg, ptrdiff_t);
            } else {
              value = va_arg(arg, unsigned int);
            }
            print_int(minus, plus, space, zero, apostrophe, hash, width, precision, value, 10, false, callback);
            break;
          }
          case 'x':
          case 'X': {
            if (width == -1) {
              width = va_arg(arg, int);
            }
            if (precision == -1) {
              precision = va_arg(arg, int);
            }

            uintmax_t value;
            if (hh) {
              value = static_cast<char>(va_arg(arg, int));
            } else if (h) {
              value = static_cast<short>(va_arg(arg, int));
            } else if (l) {
              value = va_arg(arg, long);
            } else if (ll) {
              value = va_arg(arg, long long);
            } else if (z) {
              value = va_arg(arg, size_t);
            } else if (j) {
              value = va_arg(arg, intmax_t);
            } else if (t) {
              value = va_arg(arg, ptrdiff_t);
            } else {
              value = va_arg(arg, int);
            }
            print_int(minus, plus, space, zero, apostrophe, hash, width, precision, value, 16, *p == 'X', callback);
            break;
          }
          case 'c':
            callback(va_arg(arg, int));
            break;
          case 'p':
            print_int(minus, plus, false, true, false, true, 16, false, va_arg(arg, uintptr_t), 16, false, callback);
            break;
          case 's': {
            if (width == -1) {
              width = va_arg(arg, int);
            }
            if (precision == -1) {
              precision = va_arg(arg, int);
            }

            const char* value = va_arg(arg, char*);
            print_str(minus, width, precision, value, callback);
            break;
          }
          case '%':
          case '\0':
            callback('%');
            break;
          default:
            callback('%');
            callback(*p);
            break;
        }
      }
    }
  };
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_PRINTF_H_
