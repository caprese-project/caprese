/**
 * @file bswapdi2.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief __bswapdi2
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstdint>

extern "C" uint64_t __bswapdi2(uint64_t value) {
  return ((value & 0xff00000000000000) >> 56) | ((value & 0x00ff000000000000) >> 40) | ((value & 0x0000ff0000000000) >> 24)
         | ((value & 0x000000ff00000000) >> 8) | ((value & 0x00000000ff000000) << 8) | ((value & 0x0000000000ff0000) << 24)
         | ((value & 0x000000000000ff00) << 40) | ((value & 0x00000000000000ff) << 56);
}
