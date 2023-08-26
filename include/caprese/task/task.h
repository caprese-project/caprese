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

#include <caprese/capability/capability.h>
#include <caprese/memory/address.h>
#include <caprese/task/types.h>

namespace caprese::task {
  constexpr uint32_t INIT_TID_GENERATION = (1 << (32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS))) - 1;

  [[nodiscard]] task_t*                  create_task();
  void                                   kill(task_t* task);
  void                                   cleanup(task_t* task);
  [[nodiscard]] task_t*                  lookup(tid_t tid);
  [[nodiscard]] task_t*                  get_current_task();
  [[nodiscard]] task_t*                  get_kernel_task();
  [[nodiscard]] memory::mapped_address_t get_root_page_table(task_t* task);
  [[nodiscard]] memory::mapped_address_t get_kernel_root_page_table();
} // namespace caprese::task

#endif // CAPRESE_TASK_TASK_H_
