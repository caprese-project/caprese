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

#include <bit>
#include <cstddef>
#include <cstdint>

#include <caprese/arch/task.h>
#include <caprese/memory/address.h>

namespace caprese::task {
  constexpr uint32_t INIT_TID_GENERATION = (1 << (32 - std::countr_zero<size_t>(CONFIG_MAX_TASKS))) - 1;

  struct tid_t {
    uint32_t index: std::countr_zero<size_t>(CONFIG_MAX_TASKS);
    uint32_t generation: 32 - std::countr_zero<size_t>(CONFIG_MAX_TASKS);
  };

  static_assert(sizeof(tid_t) == sizeof(uint32_t));

  constexpr uint32_t TASK_FLAG_UNUSED   = 1 << 0;
  constexpr uint32_t TASK_FLAG_CREATING = 1 << 1;
  constexpr uint32_t TASK_FLAG_RUNNING  = 1 << 2;
  constexpr uint32_t TASK_FLAG_READY    = 1 << 3;
  constexpr uint32_t TASK_FLAG_HANDLER  = 1 << 31;

  struct alignas(CONFIG_TASK_SIZE) task_t {
    tid_t        tid;
    uint32_t     flags;
    arch::task_t arch_task;
  };

  static_assert(sizeof(task_t) == CONFIG_TASK_SIZE);

  task_t*                  create_task();
  void                     switch_to(task_t* task);
  task_t*                  lookup(tid_t tid);
  task_t*                  get_current_task();
  task_t*                  get_kernel_task();
  memory::mapped_address_t get_root_page_table(task_t* task);
  memory::mapped_address_t get_kernel_root_page_table();
} // namespace caprese::task

#endif // CAPRESE_TASK_TASK_H_
