/**
 * @file panic.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare functions to safely stop the kernel in the event of a fatal condition.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RISCV_PANIC_H_
#define CAPRESE_ARCH_RISCV_PANIC_H_

#ifdef CONFIG_ARCH_RISCV

namespace caprese::arch {
  void              dump_context();
  [[noreturn]] void halt();
} // namespace caprese::arch

#endif // CONFIG_ARCH_RISCV

#endif // CAPRESE_ARCH_RISCV_PANIC_H_
