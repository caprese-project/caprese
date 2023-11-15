#ifndef KERNEL_TASK_H_
#define KERNEL_TASK_H_

#include <bit>
#include <cstddef>

#include <kernel/cap_space.h>
#include <kernel/context.h>
#include <kernel/frame.h>
#include <kernel/lock.h>
#include <kernel/page.h>

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

struct task_queue_t {
  tid_t prev;
  tid_t next;
};

enum struct task_state_t : uint8_t {
  unused    = 0,
  creating  = 1,
  running   = 2,
  ready     = 3,
  waiting   = 4,
  suspended = 5,
  killed    = 6,
};

struct alignas(PAGE_SIZE) task_t {
  context_t            context;
  frame_t              frame;
  tid_t                tid;
  cap_count_t          cap_count;
  task_queue_t         ready_queue;
  task_queue_t         waiting_queue;
  cap_slot_t*          free_slots;
  page_table_t*        root_page_table;
  task_state_t         state;
  recursive_spinlock_t lock;
  char                 stack[];
};

static_assert(sizeof(task_t) == PAGE_SIZE);

void init_task(task_t* task, cap_space_t* cap_space, page_table_t* root_page_table, page_table_t (&cap_space_page_tables)[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL]);

[[nodiscard]] bool insert_cap(task_t* task, cap_t cap);

[[nodiscard]] bool insert_cap_space(task_t* task, cap_space_t* cap_space);
[[nodiscard]] bool extend_cap_space(task_t* task, map_addr_t page);

void kill_task(task_t* task);
void switch_task(task_t* task);
void suspend_task(task_t* task);
void resume_task(task_t* task);

task_t* lookup_tid(tid_t tid);

#endif // KERNEL_TASK_H_
