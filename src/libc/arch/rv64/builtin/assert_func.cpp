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
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstdio>
#include <cstdlib>

extern "C" [[noreturn]] void __assert_func(const char* file, int line, const char* function, const char* condition) {
  printf("Assertion failed: %s, %s at %s:%d\n", condition, function, file, line);
  abort();
}
