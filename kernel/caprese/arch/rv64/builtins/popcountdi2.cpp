/**
 * @file popcountdi2.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief __popcountdi2
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#include <cstddef>
#include <cstdint>

extern "C" int __popcountdi2(uint64_t value) {
  int count = 0;
  for (size_t i = 0; i < sizeof(uint64_t); ++i) {
    if (value & (1ull << i)) {
      ++count;
    }
  }
  return count;
}
