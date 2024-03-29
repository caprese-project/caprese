#include <cassert>
#include <cerrno>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <iterator>
#include <mutex>
#include <utility>

#include <kernel/cls.h>
#include <kernel/ipc.h>
#include <kernel/lock.h>
#include <kernel/log.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <kernel/user_memory.h>
#include <libcaprese/syscall.h>

namespace {
  constexpr const char* tag = "kernel/task";

  spinlock_t next_tid_lock;
  spinlock_t lookup_tid_lock;
  uint32_t   cur_tid = 0;
  jmp_buf    jump_buffer;

  task_queue_t         ready_queue;
  recursive_spinlock_t ready_queue_lock;

  tid_t next_tid() {
    std::lock_guard lock(next_tid_lock);
    ++cur_tid;
    return std::bit_cast<tid_t>(cur_tid);
  }
} // namespace

void init_task(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space, map_ptr<page_table_t> root_page_table, map_ptr<page_table_t> (&cap_space_page_tables)[NUM_INTER_PAGE_TABLE + 1]) {
  assert(task != nullptr);
  assert(cap_space != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  task->tid = next_tid();
  memset(&task->lock, 0, sizeof(task->lock));

  std::lock_guard lock(task->lock);

  task->cap_count         = {};
  task->prev_ready_task   = 0_map;
  task->next_ready_task   = 0_map;
  task->prev_waiting_task = 0_map;
  task->next_waiting_task = 0_map;
  task->caller_task       = 0_map;
  task->callee_task       = 0_map;
  task->free_slots        = 0_map;
  task->free_slots_count  = 0;
  task->root_page_table   = root_page_table;
  task->kill_notify       = 0_map;
  task->state             = task_state_t::suspended;
  task->ipc_state         = ipc_state_t::none;
  task->ipc_msg_state     = ipc_msg_state_t::empty;
  task->event_type        = event_type_t::none;
  task->exit_status       = 0;

  memset(root_page_table.get(), 0, sizeof(page_table_t));

  constexpr size_t max_page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(max_page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += max_page_size) {
    map_ptr<pte_t> pte = root_page_table->walk(make_virt_ptr(CONFIG_MAPPED_SPACE_BASE + phys), MAX_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 0, .global = 1 });
    pte->set_next_page(make_phys_ptr(phys));
    pte->enable();
  }

  for (auto& table : cap_space_page_tables) {
    memset(table.get(), 0, sizeof(page_table_t));
  }

  map_ptr<page_table_t> page_table = root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE), level);
    assert(pte->is_disabled());
    pte->set_next_page(cap_space_page_tables[level - 1].as<void>());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
    pte->enable();
    page_table = pte->get_next_page().as<page_table_t>();
  }
  if (extend_cap_space(task, cap_space_page_tables[0]) == nullptr) {
    panic("Failed to extend cap space.");
  }

  if (!insert_cap_space(task, cap_space)) [[unlikely]] {
    panic("Failed to insert cap space.");
  }

  arch_init_task(task, return_to_user_mode);
}

void init_idle_task(map_ptr<task_t> task, map_ptr<page_table_t> root_page_table) {
  assert(task != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  std::lock_guard lock(task->lock);

  task->cap_count         = {};
  task->prev_ready_task   = 0_map;
  task->next_ready_task   = 0_map;
  task->prev_waiting_task = 0_map;
  task->next_waiting_task = 0_map;
  task->free_slots        = 0_map;
  task->root_page_table   = root_page_table;
  task->state             = task_state_t::ready;

  memset(root_page_table.get(), 0, sizeof(page_table_t));

  constexpr size_t max_page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(max_page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += max_page_size) {
    map_ptr<pte_t> pte = root_page_table->walk(make_virt_ptr(CONFIG_MAPPED_SPACE_BASE + phys), MAX_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 0, .global = 1 });
    pte->set_next_page(make_phys_ptr(phys));
    pte->enable();
  }

  arch_init_task(task, idle);
}

map_ptr<cap_slot_t> insert_cap(map_ptr<task_t> task, capability_t cap) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  map_ptr<cap_slot_t> slot = pop_free_slots(task);
  if (slot == nullptr) [[unlikely]] {
    errno = SYS_E_OUT_OF_CAP_SPACE;
    return 0_map;
  }
  slot->cap = cap;

  return slot;
}

void push_free_slots(map_ptr<task_t> task, map_ptr<cap_slot_t> slot) {
  assert(task != nullptr);
  assert(slot != nullptr);
  assert(slot->get_cap_space()->meta_info.task == task);

  std::lock_guard lock(task->lock);

  slot->erase_this();
  slot->cap = make_null_cap();

  if (task->free_slots != nullptr) {
    task->free_slots->insert_before(slot);
  }

  task->free_slots = slot;

  ++task->free_slots_count;
}

[[nodiscard]] map_ptr<cap_slot_t> pop_free_slots(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  map_ptr<cap_slot_t> slot = task->free_slots;

  if (slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to pop from the free slot. The free slot is empty.");
    errno = SYS_E_OUT_OF_CAP_SPACE;
    return 0_map;
  }

  task->free_slots = slot->erase_this();

  --task->free_slots_count;

  return slot;
}

void kill_task(map_ptr<task_t> task, int exit_status) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  assert(task->state != task_state_t::unused);

  switch (task->state) {
    case task_state_t::ready:
      remove_ready_queue(task);
      break;
    case task_state_t::waiting:
      if (task->endpoint != nullptr) {
        std::lock_guard ep_lock(task->endpoint->lock);

        switch (task->ipc_state) {
          case ipc_state_t::sending:
            remove_waiting_queue(task->endpoint->sender_queue, task);
            break;
          case ipc_state_t::receiving:
            remove_waiting_queue(task->endpoint->receiver_queue, task);
            break;
          case ipc_state_t::calling: {
            std::lock_guard callee_lock(task->callee_task->lock);
            assert(task->callee_task->caller_task == task);
            task->callee_task->caller_task = 0_map;
            break;
          }
          default:
            panic("Unexpected ipc state: %s", ipc_state_to_str(task->ipc_state));
        }
      }
      break;
    default:
      break;
  }

  // TODO: Release caps

  task->state       = task_state_t::killed;
  task->exit_status = exit_status;

  if (task->kill_notify != nullptr) {
    ipc_send_kill_notify(task->kill_notify, task);
  }

  logd(tag, "Task 0x%x has been killed. status: %d", task->tid, exit_status);

  if (task->tid.index == 1) [[unlikely]] {
    panic("The root task has been killed.");
  }

  if (task == get_cls()->current_task) [[unlikely]] {
    resched();
    std::unreachable();
  }
}

void switch_task(map_ptr<task_t> task) {
  assert(task != nullptr);
  assert(task->state == task_state_t::ready);

  map_ptr<task_t> old_task = get_cls()->current_task;

  if (task == old_task) [[unlikely]] {
    logw(tag, "Tried to switch to oneself. It's a meaningless operation.");
    return;
  }

  task->state             = task_state_t::running;
  get_cls()->current_task = task;
  old_task->state         = task_state_t::ready;
  push_ready_queue(old_task);
  switch_context(make_map_ptr(&task->context), make_map_ptr(&old_task->context));
  assert(old_task->state == task_state_t::running);
  assert(get_cls()->current_task == old_task);
}

void suspend_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  switch (task->state) {
    case task_state_t::running:
      task->state = task_state_t::suspended;
      if (task == get_cls()->current_task) {
        resched();
      } else {
        // TODO: ipi
      }
      break;
    case task_state_t::ready:
      remove_ready_queue(task);
      task->state = task_state_t::suspended;
      break;
    case task_state_t::waiting:
      task->state = task_state_t::suspended;
      break;
    default:
      break;
  }
}

void resume_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state != task_state_t::suspended) [[unlikely]] {
    logd(tag, "Failed to resume task. The task is not suspended.");
    return;
  }

  task->state = task_state_t::ready;
  push_ready_queue(task);
}

void set_kill_notify(map_ptr<task_t> task, map_ptr<endpoint_t> ep) {
  assert(task != nullptr);
  assert(ep != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return;
  }

  task->kill_notify = ep;
}

void push_ready_queue(map_ptr<task_t> task) {
  assert(task != nullptr);
  assert(task->state == task_state_t::ready);
  assert(task->prev_ready_task == nullptr);
  assert(task->next_ready_task == nullptr);

  std::lock_guard lock(ready_queue_lock);

  if (ready_queue.head == nullptr) {
    ready_queue.head = task;
    ready_queue.tail = task;
  } else {
    ready_queue.tail->next_ready_task = task;
    task->prev_ready_task             = ready_queue.tail;
    ready_queue.tail                  = task;
  }
}

void remove_ready_queue(map_ptr<task_t> task) {
  assert(task != nullptr);
  assert(task->state == task_state_t::ready);

  std::lock_guard lock(ready_queue_lock);

  if (ready_queue.head == task) {
    ready_queue.head = task->next_ready_task;
  } else {
    task->prev_ready_task->next_ready_task = task->next_ready_task;
  }

  if (ready_queue.tail == task) {
    ready_queue.tail = task->prev_ready_task;
  } else {
    task->next_ready_task->prev_ready_task = task->prev_ready_task;
  }

  task->prev_ready_task = 0_map;
  task->next_ready_task = 0_map;
}

map_ptr<task_t> pop_ready_task() {
  std::lock_guard lock(ready_queue_lock);

  map_ptr<task_t> task = ready_queue.head;
  if (task == nullptr) [[unlikely]] {
    return 0_map;
  }

  remove_ready_queue(task);
  return task;
}

extern "C" {
  static void lookup_tid_signal_handler([[maybe_unused]] int) {
    longjmp(jump_buffer, 1);
  }
}

map_ptr<task_t> lookup_tid(tid_t tid) {
  if (tid.index == 0) [[unlikely]] {
    return 0_map;
  }

  std::lock_guard lock(lookup_tid_lock);

  auto old_handler = signal(SIGSEGV, lookup_tid_signal_handler);
  if (old_handler == SIG_ERR) [[unlikely]] {
    panic("Failed to set signal handler.");
  }

  // If the task's area is not mapped, accessing it will cause a page fault, resulting in a SEGV.
  // The signal handler for SEGV is set to 'lookup_tid_signal_handler', which uses 'longjmp' to safely return control, subsequently setting 'task' to 'nullptr' in the 'else' branch of the 'setjmp' check.

  map_ptr<task_t> task = map_ptr<task_t>::from(CONFIG_TASK_SPACE_BASE) + tid.index;
  if (setjmp(jump_buffer) == 0) {
    if (task->state == task_state_t::unused) [[unlikely]] {
      task = 0_map;
    }
  } else {
    task = 0_map;
  }

  signal(SIGSEGV, old_handler);
  return task;
}

void resched() {
  map_ptr<task_t> cur_task  = get_cls()->current_task;
  map_ptr<task_t> idle_task = get_cls()->idle_task;
  get_cls()->current_task   = idle_task;
  switch_context(make_map_ptr(&idle_task->context), make_map_ptr(&cur_task->context));
}

void yield() {
  {
    map_ptr<task_t> cur_task = get_cls()->current_task;
    std::lock_guard lock(cur_task->lock);
    cur_task->state = task_state_t::ready;
    push_ready_queue(cur_task);
  }
  resched();
}

void idle() {
  while (true) {
    map_ptr<task_t> task = pop_ready_task();
    if (task == nullptr) {
      // TODO: wait for interrupt.
      continue;
    }
    get_cls()->current_task = task;
    task->state             = task_state_t::running;
    switch_context(make_map_ptr(&task->context), make_map_ptr(&get_cls()->idle_task->context));
  }
}
