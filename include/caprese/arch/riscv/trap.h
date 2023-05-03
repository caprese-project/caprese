/**
 * @file trap.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare functions related to traps.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RISCV_TRAP_H_
#define CAPRESE_ARCH_RISCV_TRAP_H_

#include <cstdint>

#include <caprese/arch/riscv/page_table.h>

extern "C" {
  extern void* begin_of_trampoline;
  extern void* end_of_trampoline;

  extern void* trampoline_user_vector;
  extern void* trampoline_return_to_user_mode;

  extern void kernel_vector();
}

namespace caprese::arch {
  using trampoline_user_vector_t         = void (*)(uintptr_t);
  using trampoline_return_to_user_mode_t = void (*)(uintptr_t);

  constexpr uintptr_t TRAMPOLINE_BASE_ADDRESS = MAX_VIRTUAL_ADDRESS - PAGE_SIZE;
  constexpr uintptr_t TRAP_FRAME_BASE_ADDRESS = TRAMPOLINE_BASE_ADDRESS - PAGE_SIZE;

  enum struct riscv_trap_code : uintptr_t {
    INSTRUCTION_ADDRESS_MISALIGNED = 0,
    INSTRUCTION_ACCESS_FAULT       = 1,
    ILLEGAL_INSTRUCTION            = 2,
    BREAKPOINT                     = 3,
    LOAD_ADDRESS_MISALIGNED        = 4,
    LOAD_ACCESS_FAULT              = 5,
    STORE_AMO_ADDRESS_MISALIGNED   = 6,
    STORE_AMO_ACCESS_FAULT         = 7,
    ENVIRONMENT_CALL_FROM_U_MODE   = 8,
    ENVIRONMENT_CALL_FROM_S_MODE   = 9,
    ENVIRONMENT_CALL_FROM_M_MODE   = 11,
    INSTRUCTION_PAGE_FAULT         = 12,
    LOAD_PAGE_FAULT                = 13,
    STORE_AMO_PAGE_FAULT           = 15,
  };

  void return_to_user_mode();
} // namespace caprese::arch

#endif // CAPRESE_ARCH_RISCV_TRAP_H_
