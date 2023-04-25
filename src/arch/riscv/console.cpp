/**
 * @file console.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Console output
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */
#include <cstdint>

namespace caprese::arch {
  void console_put(char c) {
    asm volatile("mv a0, %0" : : "r"(static_cast<uintptr_t>(c)));
    asm volatile("li a7, 1");
    asm volatile("ecall");
  }
}
