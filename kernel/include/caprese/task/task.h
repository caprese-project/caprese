/**
 * @file task.h
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

#ifndef CAPRESE_TASK_TASK_H_
#define CAPRESE_TASK_TASK_H_

#include <cstdint>
#include <cstring>

#include <caprese/arch/common/memory/layout.h>
#include <caprese/arch/common/task/task.h>
#include <caprese/cpu/cpu_info.h>

namespace caprese::task {
  struct task_id_t {
    uint32_t task_space_id;
    uint16_t generation;
    uint16_t system_id;
  };

  static_assert(sizeof(task_id_t) == sizeof(uint64_t));

  union capability_t;
  struct capability_handle_t;

  struct task_t {
    task_id_t task_id;
    uint32_t  front_capability_space_id;
    alignas(16) arch::task::task_t arch_task;
    alignas(memory::page_size()) char kernel_stack[memory::page_size()];
  };

  static_assert(offsetof(task_t, arch_task) == 16);
  static_assert(offsetof(task_t, kernel_stack) == memory::page_size());

  void init_task_space(memory::virtual_address_t root_page_table);

  constexpr task_t* get_task_by_id(uint32_t task_space_id) {
    return reinterpret_cast<task_t*>(arch::memory::begin_of_task_space) + task_space_id;
  }

  constexpr task_t* get_task_by_id(task_id_t task_id) {
    return get_task_by_id(task_id.task_space_id);
  }

  inline void set_current_task(uint32_t task_space_id) {
    cpu::get_cpu_local_info()->current_task_space_id = task_space_id;
  }

  inline task_t* get_current_task() {
    return get_task_by_id(cpu::get_cpu_local_info()->current_task_space_id);
  }

  inline task_t* get_kernel_task() {
    return get_task_by_id(0);
  }

  task_id_t create_new_task();

  inline void switch_to(task_t* task) {
    auto old_task = get_current_task();
    set_current_task(task->task_id.task_space_id);
    arch::task::switch_context(&old_task->arch_task, &task->arch_task);
  }
} // namespace caprese::task

#endif // CAPRESE_TASK_TASK_H_
