/**
 * @file panic.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement a function to safely stop the kernel in the event of a fatal condition.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <caprese/kernel/panic.h>
#include <caprese/lib/console.h>

namespace caprese {
  [[noreturn]] void panic(const char* msg) {
    println("Kernel panic! %s", msg);
    arch::halt();
  }
}
