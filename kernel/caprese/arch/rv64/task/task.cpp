/**
 * @file task.cpp
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

#include <cassert>
#include <cstdio>

#include <caprese/arch/common/trap/trap.h>
#include <caprese/arch/rv64/task/task.h>
#include <caprese/task/task.h>

namespace caprese::arch::task {
  void task_t::setup(caprese::memory::virtual_address_t entry_address) {
    assert(satp != 0);
    assert(offsetof(caprese::task::task_t, arch_task) == (reinterpret_cast<uintptr_t>(this) & caprese::memory::page_mask()));

    auto task = reinterpret_cast<caprese::task::task_t*>(reinterpret_cast<uintptr_t>(this) & ~caprese::memory::page_mask());

    for (const char* c = begin_of_trampoline; c < end_of_trampoline; ++c) {
      trampoline[c - begin_of_trampoline] = *c;
    }

    context.ra = reinterpret_cast<uintptr_t>(trap::return_to_user_mode);
    context.sp = reinterpret_cast<uintptr_t>(task->kernel_stack);

    trap_frame.epc = entry_address.as<uintptr_t>();
    trap_frame.a0  = 0;
  }
} // namespace caprese::arch::task
