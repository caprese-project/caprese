#include <cassert>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <iterator>
#include <mutex>

#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <log/log.h>

namespace {
  spinlock_t next_tid_lock;
  spinlock_t lookup_tid_lock;
  uint32_t   cur_tid = 0;
  jmp_buf    jump_buffer;

  tid_t next_tid() {
    std::lock_guard<spinlock_t> lock(next_tid_lock);
    ++cur_tid;
    return std::bit_cast<tid_t>(cur_tid);
  }
} // namespace

void init_task(map_ptr<task_t>       task,
               map_ptr<cap_space_t>  cap_space,
               map_ptr<page_table_t> root_page_table,
               map_ptr<page_table_t> (&cap_space_page_tables)[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL]) {
  assert(task != nullptr);
  assert(cap_space != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  task->tid = next_tid();

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  task->cap_count       = {};
  task->ready_queue     = {};
  task->waiting_queue   = {};
  task->free_slots      = 0_map;
  task->root_page_table = root_page_table;
  task->state           = task_state_t::creating;

  memset(root_page_table.get(), 0, sizeof(page_table_t));

  constexpr size_t page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += page_size) {
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
    pte->set_next_page(cap_space_page_tables[level - MEGA_PAGE_TABLE_LEVEL].as<void>());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
    pte->enable();
    page_table = pte->get_next_page().as<page_table_t>();
  }
  if (!extend_cap_space(task, cap_space_page_tables[0].as<void>())) {
    panic("Failed to extend cap space.");
  }

  if (!insert_cap_space(task, cap_space)) [[unlikely]] {
    panic("Failed to insert cap space.");
  }

  arch_init_task(task);
}

map_ptr<cap_slot_t> insert_cap(map_ptr<task_t> task, capability_t cap) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->free_slots == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_slot_t> slot = task->free_slots;
  task->free_slots         = slot->prev;

  slot->prev = 0_map;
  slot->next = 0_map;
  slot->cap  = cap;

  return slot;
}

void kill_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  assert(task->state != task_state_t::unused);

  switch (task->state) {
    case task_state_t::ready:
      // TODO: impl
      break;
    case task_state_t::waiting:
      // TODO: impl
      break;
    default:
      break;
  }

  // TODO: Release caps

  task->state = task_state_t::killed;
}

void switch_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  map_ptr<task_t> old_task = get_cls()->current_task;

  if (task == old_task) [[unlikely]] {
    return;
  }

  {
    std::lock_guard<recursive_spinlock_t> lock(task->lock);
    if (task->state != task_state_t::ready) [[unlikely]] {
      return;
    }
    task->state = task_state_t::running;
  }

  get_cls()->current_task = task;
  switch_context(make_map_ptr(&task->context), make_map_ptr(&old_task->context));
  get_cls()->current_task = old_task;
}

void suspend_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  // TODO: impl
}

void resume_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->state != task_state_t::suspended) [[unlikely]] {
    return;
  }

  // TODO: impl
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

  std::lock_guard<spinlock_t> lock(lookup_tid_lock);

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
