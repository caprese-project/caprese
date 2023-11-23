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

struct task_t;

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

struct task_queue_t {
  map_ptr<task_t> head;
  map_ptr<task_t> tail;
};

struct endpoint_t {
  task_queue_t         sender_queue;
  task_queue_t         receiver_queue;
  recursive_spinlock_t lock;
};

struct alignas(PAGE_SIZE) task_t {
  context_t             context;
  frame_t               frame;
  tid_t                 tid;
  cap_count_t           cap_count;
  map_ptr<task_t>       prev_ready_task;
  map_ptr<task_t>       next_ready_task;
  map_ptr<task_t>       prev_waiting_task;
  map_ptr<task_t>       next_waiting_task;
  map_ptr<cap_slot_t>   free_slots;
  map_ptr<page_table_t> root_page_table;
  map_ptr<endpoint_t>   waiting_queue_head;
  message_buffer_t      msg_buf;
  recursive_spinlock_t  lock;
  task_state_t          state;
  int                   exit_status;
  char                  stack[];
};

static_assert(sizeof(task_t) == PAGE_SIZE);

void init_task(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space, map_ptr<page_table_t> root_page_table, map_ptr<page_table_t> (&cap_space_page_tables)[NUM_INTER_PAGE_TABLE + 1]);

void init_idle_task(map_ptr<task_t> task, map_ptr<page_table_t> root_page_table);

[[nodiscard]] map_ptr<cap_slot_t> insert_cap(map_ptr<task_t> task, capability_t cap);
[[nodiscard]] map_ptr<cap_slot_t> transfer_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot);
[[nodiscard]] map_ptr<cap_slot_t> delegate_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot);
[[nodiscard]] map_ptr<cap_slot_t> copy_cap(map_ptr<cap_slot_t> src_slot);
[[nodiscard]] bool                revoke_cap(map_ptr<cap_slot_t> slot);

void kill_task(map_ptr<task_t> task, int exit_status);
void switch_task(map_ptr<task_t> task);
void suspend_task(map_ptr<task_t> task);
void resume_task(map_ptr<task_t> task);

bool ipc_send_short(bool blocking, map_ptr<endpoint_t> endpoint, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
bool ipc_send_long(bool blocking, map_ptr<endpoint_t> endpoint);
bool ipc_receive(bool blocking, map_ptr<endpoint_t> endpoint);

bool ipc_transfer_msg(map_ptr<task_t> dst, map_ptr<task_t> src);

[[nodiscard]] map_ptr<task_t> lookup_task(tid_t tid);
[[nodiscard]] map_ptr<task_t> lookup_tid(tid_t tid);

void resched();
void yield();

void idle();

#endif // KERNEL_TASK_H_
