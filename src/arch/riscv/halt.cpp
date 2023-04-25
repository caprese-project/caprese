/**
 * @file halt.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Halt
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <caprese/kernel/panic.h>

namespace caprese::arch {
  [[noreturn]] void halt() {
    while(true) {
      asm volatile("wfi");
    }
  }
}
