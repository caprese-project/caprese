/**
 * @file cpu_info.h
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

#ifndef CAPRESE_CPU_CPU_INFO_H_
#define CAPRESE_CPU_CPU_INFO_H_

#include <cstddef>
#include <cstdint>

#include <caprese/arch/common/cpu/cpuid.h>

namespace caprese::cpu {
  struct cpu_local_info_t {
    arch::cpu::cpuid_t cpuid;
    uint32_t           current_task_space_id;
  };

  void              setup(arch::cpu::cpuid_t cpuid, size_t ncpu);
  cpu_local_info_t* get_cpu_local_info();
} // namespace caprese::cpu

#endif // CAPRESE_CPU_CPU_INFO_H_
