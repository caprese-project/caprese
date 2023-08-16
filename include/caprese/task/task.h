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
#include <cstdint>

#include <caprese/arch/task.h>
#include <caprese/capability/capability.h>
#include <caprese/memory/address.h>

namespace caprese::task {
  constexpr uint32_t INIT_TID_GENERATION = (1 << (32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS))) - 1;

  struct tid_t {
    uint32_t index: std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
    uint32_t generation: 32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
  };

  static_assert(sizeof(tid_t) == sizeof(uint32_t));

  constexpr uint8_t TASK_FLAG_UNUSED   = 1 << 0;
  constexpr uint8_t TASK_FLAG_CREATING = 1 << 1;
  constexpr uint8_t TASK_FLAG_RUNNING  = 1 << 2;
  constexpr uint8_t TASK_FLAG_READY    = 1 << 3;

  using cap_list_index_t = capability::cap_ref_t;

  struct alignas(CONFIG_TASK_SIZE) task_t {
    tid_t            tid;
    cap_list_index_t free_cap_list: capability::CAP_REF_SIZE_BIT;
    uint32_t         used_cap_space_count;
    uint8_t          flags;
    arch::task_t     arch_task;
  };

  static_assert(sizeof(task_t) == CONFIG_TASK_SIZE);
  static_assert(offsetof(task_t, arch_task) == CONFIG_ARCH_TASK_OFFSET);

  [[nodiscard]] task_t*                   create_task();
  void                                    kill(task_t* task);
  void                                    switch_to(task_t* task);
  [[nodiscard]] task_t*                   lookup(tid_t tid);
  [[nodiscard]] task_t*                   get_current_task();
  [[nodiscard]] task_t*                   get_kernel_task();
  [[nodiscard]] memory::mapped_address_t  get_root_page_table(task_t* task);
  [[nodiscard]] memory::mapped_address_t  get_kernel_root_page_table();
  [[nodiscard]] capability::capability_t* lookup_capability(task_t* task, capability::cid_t cid);
  cap_list_index_t                        insert_capability(task_t* task, capability::capability_t* cap);
  capability::cid_t                       move_capability(task_t* dst_task, task_t* src_task, cap_list_index_t index);
  [[nodiscard]] capability::cid_t*        get_cid(task_t* task, cap_list_index_t index);
  cap_list_index_t                        allocated_cap_list_size(task_t* task);
} // namespace caprese::task

#endif // CAPRESE_TASK_TASK_H_
