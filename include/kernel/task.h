#ifndef KERNEL_TASK_H_
#define KERNEL_TASK_H_

#include <bit>
#include <cstddef>

#include <kernel/cap.h>
#include <kernel/cap_space.h>
#include <kernel/context.h>
#include <kernel/frame.h>
#include <kernel/lock.h>
#include <kernel/page.h>
#include <libcaprese/ipc.h>

struct tid_t {
  uint32_t index: std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
  uint32_t generation: 32 - std::countr_zero<uintptr_t>(CONFIG_MAX_TASKS);
};

static_assert(sizeof(tid_t) == sizeof(uint32_t));

constexpr bool operator==(const tid_t& lhs, const tid_t& rhs) {
  return std::bit_cast<uint32_t>(lhs) == std::bit_cast<uint32_t>(rhs);
}

constexpr bool operator!=(const tid_t& lhs, const tid_t& rhs) {
  return std::bit_cast<uint32_t>(lhs) != std::bit_cast<uint32_t>(rhs);
}

struct cap_count_t {
  uint32_t num_cap_space: std::countr_zero(NUM_PAGE_TABLE_ENTRY* NUM_PAGE_TABLE_ENTRY);
  uint32_t num_extension: std::countr_zero(NUM_PAGE_TABLE_ENTRY);
};

enum struct task_state_t : uint8_t {
  unused    = 0,
  running   = 1,
  ready     = 2,
  waiting   = 3,
  suspended = 4,
  killed    = 5,
};

enum struct ipc_state_t : uint8_t {
  none      = 0,
  sending   = 1,
  receiving = 2,
  calling   = 3,
  canceled  = 4,
};

enum struct event_type_t : uint8_t {
  none = 0,
  send = 1,
  kill = 2,
};

enum struct ipc_msg_state_t : uint8_t {
  empty      = 0,
  short_size = 1,
  long_size  = 2,
};

inline const char* task_state_to_str(task_state_t state) {
  switch (state) {
    case task_state_t::unused:
      return "unused";
    case task_state_t::running:
      return "running";
    case task_state_t::ready:
      return "ready";
    case task_state_t::waiting:
      return "waiting";
    case task_state_t::suspended:
      return "suspended";
    case task_state_t::killed:
      return "killed";
    default:
      return "unknown";
  }
}

inline const char* ipc_state_to_str(ipc_state_t state) {
  switch (state) {
    case ipc_state_t::none:
      return "none";
    case ipc_state_t::sending:
      return "sending";
    case ipc_state_t::receiving:
      return "receiving";
    case ipc_state_t::calling:
      return "calling";
    case ipc_state_t::canceled:
      return "canceled";
    default:
      return "unknown";
  }
}

struct alignas(PAGE_SIZE) task_t {
  context_t             context;
  frame_t               frame;
  tid_t                 tid;
  cap_count_t           cap_count;
  map_ptr<task_t>       prev_ready_task;
  map_ptr<task_t>       next_ready_task;
  map_ptr<task_t>       prev_waiting_task;
  map_ptr<task_t>       next_waiting_task;
  map_ptr<task_t>       caller_task;
  map_ptr<task_t>       callee_task;
  map_ptr<cap_slot_t>   free_slots;
  size_t                free_slots_count;
  map_ptr<page_table_t> root_page_table;
  map_ptr<endpoint_t>   endpoint;
  map_ptr<endpoint_t>   kill_notify;
  recursive_spinlock_t  lock;

  union {
    uintptr_t           ipc_short_msg[6];
    virt_ptr<message_t> ipc_long_msg;
  };

  task_state_t    state;
  ipc_state_t     ipc_state;
  ipc_msg_state_t ipc_msg_state;
  event_type_t    event_type;
  int             exit_status;
  char            stack[];
};

static_assert(sizeof(task_t) == PAGE_SIZE);

void init_task(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space, map_ptr<page_table_t> root_page_table, map_ptr<page_table_t> (&cap_space_page_tables)[NUM_INTER_PAGE_TABLE + 1]);

void init_idle_task(map_ptr<task_t> task, map_ptr<page_table_t> root_page_table);

[[nodiscard]] map_ptr<cap_slot_t> insert_cap(map_ptr<task_t> task, capability_t cap);
void                              push_free_slots(map_ptr<task_t> task, map_ptr<cap_slot_t> slot);
[[nodiscard]] map_ptr<cap_slot_t> pop_free_slots(map_ptr<task_t> task);

void kill_task(map_ptr<task_t> task, int exit_status);
void switch_task(map_ptr<task_t> task);
void suspend_task(map_ptr<task_t> task);
void resume_task(map_ptr<task_t> task);

void set_kill_notify(map_ptr<task_t> task, map_ptr<endpoint_t> ep);

void            push_ready_queue(map_ptr<task_t> task);
void            remove_ready_queue(map_ptr<task_t> task);
map_ptr<task_t> pop_ready_task();

[[nodiscard]] map_ptr<task_t> lookup_tid(tid_t tid);

void resched();
void yield();

void idle();

#endif // KERNEL_TASK_H_
