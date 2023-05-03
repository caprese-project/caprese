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

  constexpr uintptr_t trampoline_base_address = max_virtual_address - page_size;
  constexpr uintptr_t trap_frame_base_address = trampoline_base_address - page_size;

  enum struct riscv_trap_code : uintptr_t {
    instruction_address_misaligned = 0,
    instruction_access_fault       = 1,
    illegal_instruction            = 2,
    breakpoint                     = 3,
    load_address_misaligned        = 4,
    load_access_fault              = 5,
    store_amo_address_misaligned   = 6,
    store_amo_access_fault         = 7,
    environment_call_from_u_mode   = 8,
    environment_call_from_s_mode   = 9,
    environment_call_from_m_mode   = 11,
    instruction_page_fault         = 12,
    load_page_fault                = 13,
    store_amo_page_fault           = 15,
  };

  void return_to_user_mode();
} // namespace caprese::arch

#endif // CAPRESE_ARCH_RISCV_TRAP_H_
