/**
 * @file panic.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare a function to safely stop the kernel in the event of a fatal condition.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#ifndef CAPRESE_KERNEL_PANIC_H_
#define CAPRESE_KERNEL_PANIC_H_

namespace caprese {
  namespace arch {
    [[noreturn]] void halt();
  }
  [[noreturn]] void panic(const char* msg);
}

#endif // CAPRESE_KERNEL_PANIC_H_
