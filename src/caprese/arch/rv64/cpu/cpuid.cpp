/**
 * @file cpuid.cpp
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

#include <caprese/arch/common/cpu/cpuid.h>
#include <caprese/cpu/cpu_info.h>

namespace caprese::arch::cpu {
  void set_cpuid(cpuid_t cpuid) {
    asm volatile("mv tp, %0" : : "r"(cpuid));
  }

  cpuid_t get_cpuid() {
    cpuid_t cpuid;
    asm volatile("mv %0, tp" : "=r"(cpuid));
    return cpuid;
  }
} // namespace caprese::arch::cpu
