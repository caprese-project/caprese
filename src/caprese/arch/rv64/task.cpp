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

#include <cstdlib>

#include <caprese/arch/memory.h>
#include <caprese/arch/rv64/task.h>

extern "C" {
  extern void _jump_to_kernel_entry();
  extern void _switch_context(caprese::arch::task_t* old_task, caprese::arch::task_t* new_task);
  extern void _load_context([[maybe_unused]] nullptr_t, caprese::arch::task_t* task);
}

namespace caprese::arch::inline rv64 {
  void create_kernel_task(task_t* task, void (*entry)(boot_info_t*), boot_info_t* boot_info) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(_jump_to_kernel_entry);
    task->context.sp = reinterpret_cast<uint64_t>(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE)) + arch::PAGE_SIZE;
    task->context.s0 = reinterpret_cast<uint64_t>(entry);
    task->context.s1 = reinterpret_cast<uint64_t>(boot_info);
  }

  void switch_context(task_t* old_task, task_t* new_task) {
    _switch_context(old_task, new_task);
  }

  void load_context(task_t* task) {
    _load_context(nullptr, task);
  }
} // namespace caprese::arch::inline rv64
