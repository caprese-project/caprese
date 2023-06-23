/**
 * @file layout.h
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

#ifndef CAPRESE_ARCH_RV64_MEMORY_LAYOUT_H_
#define CAPRESE_ARCH_RV64_MEMORY_LAYOUT_H_

#include <cstdint>

namespace caprese::arch::memory {
  constexpr uintptr_t begin_of_user_mode_address_space   = 0x0000'0000'0000ull;
  constexpr uintptr_t end_of_user_mode_address_space     = 0x3FFF'FFFF'FFFFull;
  constexpr uintptr_t begin_of_kernel_mode_address_space = 0x4000'0000'0000ull;
  constexpr uintptr_t end_of_kernel_mode_address_space   = 0x7FFF'FFFF'FFFFull;
  constexpr uintptr_t begin_of_capability_space          = 0x4000'0000'0000ull;
  constexpr uintptr_t end_of_capability_space            = 0x4FFF'FFFF'FFFFull;
  constexpr uintptr_t begin_of_task_space                = 0x5000'0000'0000ull;
  constexpr uintptr_t end_of_task_space                  = 0x6FFF'FFFF'FFFFull;
  constexpr uintptr_t begin_of_phys_map_space            = 0x7000'0000'0000ull;
  constexpr uintptr_t end_of_phys_map_space              = 0x77FF'FFFF'FFFFull;
  constexpr uintptr_t begin_of_kernel_code_space         = 0x7800'0000'0000ull;
  constexpr uintptr_t end_of_kernel_code_space           = 0x7800'0FFF'FFFFull;
  constexpr uintptr_t begin_of_cpu_local_space           = 0x7800'1000'0000ull;
  constexpr uintptr_t end_of_cpu_local_space             = 0x7800'1007'FFFFull;
  constexpr uintptr_t begin_of_kernel_heap_space         = 0x7800'2000'0000ull;
  constexpr uintptr_t end_of_kernel_heap_space           = 0x7800'2FFF'FFFFull;
} // namespace caprese::arch

#endif // CAPRESE_ARCH_RV64_MEMORY_LAYOUT_H_
