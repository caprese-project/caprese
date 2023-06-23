/**
 * @file cpu_info.cpp
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
#include <caprese/arch/common/memory/layout.h>
#include <caprese/cpu/cpu_info.h>
#include <caprese/memory/page.h>
#include <caprese/memory/page_stack.h>
#include <caprese/task/task.h>

#include <libcaprese/util/enum.h>

namespace caprese::cpu {
  void setup(arch::cpu::cpuid_t cpuid, size_t ncpu) {
    arch::cpu::set_cpuid(cpuid);

    for (size_t i = 0; i < ncpu; ++i) {
      memory::map(task::get_task_by_id(0)->arch_task.get_root_page_table(),
                  arch::memory::begin_of_cpu_local_space + i * 0x1000,
                  memory::virt_to_phys(capability::get_capability(memory::pop_page()).memory.address()),
                  memory::page_flag::readable | memory::page_flag::writable | memory::page_flag::global);
    }

    get_cpu_local_info()->cpuid = cpuid;
  }

  cpu_local_info_t* get_cpu_local_info() {
    auto cpuid = arch::cpu::get_cpuid();
    return reinterpret_cast<cpu_local_info_t*>(arch::memory::begin_of_cpu_local_space + cpuid * 0x1000);
  }
} // namespace caprese::cpu
