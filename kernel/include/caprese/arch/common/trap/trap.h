/**
 * @file trap.h
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

#ifndef CAPRESE_ARCH_COMMON_TRAP_TRAP_H_
#define CAPRESE_ARCH_COMMON_TRAP_TRAP_H_

#ifdef CONFIG_ARCH_RISCV64
#include <caprese/arch/rv64/trap/trap.h>
#else
#error "Unknown Arch"
#endif

namespace caprese::arch::trap {
  void return_to_user_mode();
} // namespace caprese::arch

#endif // CAPRESE_ARCH_COMMON_TRAP_TRAP_H_
