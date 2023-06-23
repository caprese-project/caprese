/**
 * @file cpuid.h
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

#ifndef CAPRESE_ARCH_COMMON_CPU_CPUID_H_
#define CAPRESE_ARCH_COMMON_CPU_CPUID_H_

#include <cstdint>

namespace caprese::arch::cpu {
  using cpuid_t = uintptr_t;

  void    set_cpuid(cpuid_t cpuid);
  cpuid_t get_cpuid();
} // namespace caprese::arch::cpu

#endif // CAPRESE_ARCH_COMMON_CPU_CPUID_H_
