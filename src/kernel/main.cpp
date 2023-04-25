/**
 * @file main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement platform-independent kernel main.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <caprese/kernel/main.h>
#include <caprese/kernel/panic.h>
#include <caprese/lib/console.h>

namespace caprese {
  [[noreturn]] void main() {
    println("int: %d, uint: %u, xint: %x, ptr: %p, char: %c, str: %s, per: %%, unknown: %g", -200, 500, 128, main, 'H', "Hello, World!");
    panic("TEST PANIC");
  }
}
