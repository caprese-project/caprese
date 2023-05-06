/**
 * @file assert_func.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief __assert_func
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#include <caprese/lib/console.h>

extern "C" [[noreturn]] void __assert_func(const char* file, int line, const char* function, const char* condition) {
  caprese::log_fatal("assert", "Assertion failed: %s, %s at %s:%d", condition, function, file, line);
}
