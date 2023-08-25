#ifndef CAPRESE_TASK_TYPES_H_
#define CAPRESE_TASK_TYPES_H_

#include <cstdint>

#include <caprese/arch/task.h>
#include <caprese/capability/capability.h>

namespace caprese::task {
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

  constexpr uint32_t MSG_FLAG_TYPE_SHORT_MSG = 1;
  constexpr uint32_t MSG_FLAG_TYPE_MSG       = 2;
  constexpr uint32_t MSG_FLAG_TYPE_LONG_MSG  = 3;
  constexpr uint32_t MSG_FLAG_CANCELED       = 4;
  constexpr uint32_t MSG_FLAG_FASTPATH       = 8;
  constexpr int      MSG_FLAG_BIT            = 4;
  constexpr uint32_t MSG_FLAG_TYPE_MASK      = 0b11;

  struct msg_t {
    uint32_t flags: MSG_FLAG_BIT;
    uint32_t channel: 32 - MSG_FLAG_BIT;

    union {
      struct {
        uintptr_t data[4];
      } short_msg;

      struct {
        cid_handle_t handles[4];
      } msg;

      struct {
        size_t       msg_size;
        cid_handle_t table_handle;
      } long_msg;
    };
  };

  constexpr uint8_t TASK_STATE_UNUSED   = 0;
  constexpr uint8_t TASK_STATE_CREATING = 1;
  constexpr uint8_t TASK_STATE_RUNNING  = 2;
  constexpr uint8_t TASK_STATE_READY    = 3;
  constexpr uint8_t TASK_STATE_WAITING  = 4;
  constexpr uint8_t TASK_STATE_KILLED   = 5;

  struct alignas(CONFIG_TASK_SIZE) task_t {
    tid_t                    tid;
    cid_handle_t             free_cap_list: capability::CAP_REF_SIZE_BIT;
    uint32_t                 used_cap_space_count;
    tid_t                    prev_ready_task;
    tid_t                    next_ready_task;
    tid_t                    prev_waiting_queue;
    tid_t                    next_waiting_queue;
    tid_t                    next_sender_task;
    memory::mapped_address_t msg;
    uint8_t                  state;
    arch::task_t             arch_task;
  };

  static_assert(sizeof(task_t) == CONFIG_TASK_SIZE);
  static_assert(offsetof(task_t, arch_task) == CONFIG_ARCH_TASK_OFFSET);

  constexpr tid_t null_tid {
    .index      = 0,
    .generation = (1 << (32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS))) - 1,
  };

  constexpr bool operator==(const tid_t& lhs, const tid_t& rhs) {
    return std::bit_cast<uint32_t>(lhs) == std::bit_cast<uint32_t>(rhs);
  }

  constexpr bool operator!=(const tid_t& lhs, const tid_t& rhs) {
    return std::bit_cast<uint32_t>(lhs) != std::bit_cast<uint32_t>(rhs);
  }
} // namespace caprese::task

#endif // CAPRESE_TASK_TYPES_H_
