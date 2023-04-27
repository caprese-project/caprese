/**
 * @file console.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare a console output utility for the kernel.
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#ifndef CAPRESE_LIB_CONSOLE_H_
#define CAPRESE_LIB_CONSOLE_H_

namespace caprese {
  void print(const char* fmt, ...);
  void println(const char* fmt, ...);

  [[noreturn]] void log_fatal(const char* tag, const char* fmt, ...);
  void              log_error(const char* tag, const char* fmt, ...);
  void              log_warn(const char* tag, const char* fmt, ...);
  void              log_info(const char* tag, const char* fmt, ...);
  void              log_debug(const char* tag, const char* fmt, ...);
  void              log_assert(const char* tag, bool condition, const char* fmt, ...);
} // namespace caprese

#endif // CAPRESE_LIB_CONSOLE_H_
