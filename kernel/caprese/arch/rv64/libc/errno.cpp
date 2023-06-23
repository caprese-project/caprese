/**
 * @file errno.cpp
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

#include <cerrno>

namespace {
  int errno_value = 0;
} // namespace

extern "C" {
  int* __errno(void) {
    return &errno_value;
  }
}
