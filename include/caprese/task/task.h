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
#include <caprese/capability/capability.h>
#include <caprese/memory/address.h>

namespace caprese::task {
  constexpr uint32_t INIT_TID_GENERATION = (1 << (32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS))) - 1;

  struct tid_t {
    uint32_t index: std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
    uint32_t generation: 32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
  };

  static_assert(sizeof(tid_t) == sizeof(uint32_t));

  using cid_handle_t = uint32_t;
  static_assert(sizeof(cid_handle_t) == sizeof(capability::cap_ref_t));

  struct cid_t {
    capability::ccid_t    ccid;
    capability::cap_gen_t generation;

    union {
      // ccid != 0
      capability::cap_ref_t cap_ref: capability::CAP_REF_SIZE_BIT;

      // ccid == 0
      cid_handle_t prev_free_cap_list: capability::CAP_REF_SIZE_BIT;
    };
  };

  static_assert(sizeof(cid_t) == CONFIG_CID_SIZE);

  constexpr uint8_t TASK_STATE_UNUSED   = 0;
  constexpr uint8_t TASK_STATE_CREATING = 1;
  constexpr uint8_t TASK_STATE_RUNNING  = 2;
  constexpr uint8_t TASK_STATE_READY    = 3;

  struct alignas(CONFIG_TASK_SIZE) task_t {
    tid_t        tid;
    cid_handle_t free_cap_list: capability::CAP_REF_SIZE_BIT;
    uint32_t     used_cap_space_count;
    uint8_t      state;
    arch::task_t arch_task;
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
  [[nodiscard]] cid_t                     make_cid(capability::capability_t* cap);
  [[nodiscard]] cid_t*                    lookup_cid(task_t* task, cid_handle_t handle);
  [[nodiscard]] capability::capability_t* lookup_capability(task_t* task, cid_t cid);
  cid_handle_t                            insert_capability(task_t* task, capability::capability_t* cap);
  cid_t                                   move_capability(task_t* dst_task, task_t* src_task, cid_handle_t handle);
  size_t                                  allocated_cap_list_size(task_t* task);

  constexpr cid_t null_cid() {
    return { .ccid = 0, .generation = 0, .prev_free_cap_list = 0 };
  }
} // namespace caprese::task

#endif // CAPRESE_TASK_TASK_H_
