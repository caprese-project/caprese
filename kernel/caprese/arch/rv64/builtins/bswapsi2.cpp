/**
 * @file bswapsi2.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief __bswapsi2
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

extern "C" uint32_t __bswapsi2(uint32_t value) {
  return ((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8) | ((value & 0x0000ff00) << 8) | ((value & 0x000000ff) << 24);
}
