#include <algorithm>
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

void init_task(task_t* task, cap_space_t* cap_space, page_table_t* root_page_table, page_table_t (&cap_space_page_tables)[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL]) {
  assert(task != nullptr);
  assert(cap_space != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  task->tid = next_tid();

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  task->cap_count       = {};
  task->ready_queue     = {};
  task->waiting_queue   = {};
  task->free_slots      = nullptr;
  task->root_page_table = root_page_table;
  task->state           = task_state_t::creating;

  memset(root_page_table, 0, sizeof(page_table_t));

  constexpr size_t page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += page_size) {
    pte_t* pte = root_page_table->walk(virt_addr_t::from(CONFIG_MAPPED_SPACE_BASE + phys), MAX_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 0, .global = 1 });
    pte->set_next_page(phys_addr_t::from(phys).as_map());
    pte->enable();
  }

  memset(cap_space_page_tables, 0, sizeof(cap_space_page_tables));

  page_table_t* page_table = root_page_table;
  pte_t*        pte        = nullptr;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(virt_addr_t::from(CONFIG_CAPABILITY_SPACE_BASE), level);
    assert(pte->is_disabled());
    pte->set_next_page(map_addr_t::from(&cap_space_page_tables[level - MEGA_PAGE_TABLE_LEVEL]));
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
    pte->enable();
    page_table = pte->get_next_page().as<page_table_t*>();
  }
  if (!extend_cap_space(task, map_addr_t::from(cap_space_page_tables))) {
    panic("Failed to extend cap space.");
  }

  if (!insert_cap_space(task, cap_space)) [[unlikely]] {
    panic("Failed to insert cap space.");
  }

  arch_init_task(task);
}

bool insert_cap(task_t* task, cap_t cap) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->free_slots == nullptr) [[unlikely]] {
    return false;
  }

  cap_slot_t* slot = task->free_slots;
  task->free_slots = slot->prev;

  slot->cap = cap;

  return true;
}

bool insert_cap_space(task_t* task, cap_space_t* cap_space) {
  assert(task != nullptr);
  assert(cap_space != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->cap_count.num_cap_space / NUM_PAGE_TABLE_ENTRY > task->cap_count.num_extension) [[unlikely]] {
    return false;
  }

  virt_addr_t cap_space_base_va = virt_addr_t::from(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * task->cap_count.num_cap_space);

  page_table_t* page_table = task->root_page_table;
  pte_t*        pte        = nullptr;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= MEGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(cap_space_base_va, level);
    assert(pte->is_enabled());
    page_table = pte->get_next_page().as<page_table_t*>();
  }

  pte = page_table->walk(cap_space_base_va, KILO_PAGE_TABLE_LEVEL);
  assert(pte->is_disabled());
  pte->set_next_page(map_addr_t::from(cap_space));
  pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
  pte->enable();

  auto end = std::rend(cap_space->slots);
  if (task->cap_count.num_cap_space == 0) [[unlikely]] {
    // The first element of cap-space is always null-cap.
    --end;
    cap_space->slots[0].cap  = make_null_cap();
    cap_space->slots[0].next = nullptr;
    cap_space->slots[0].prev = nullptr;
  }

  std::for_each(std::rbegin(cap_space->slots), end, [task](auto&& slots) {
    slots.prev       = task->free_slots;
    task->free_slots = &slots;
  });

  ++task->cap_count.num_cap_space;

  return true;
}

bool extend_cap_space(task_t* task, map_addr_t page) {
  assert(task != nullptr);
  assert(page.is_aligned_to_pow2(PAGE_SIZE));

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->cap_count.num_extension == NUM_PAGE_TABLE_ENTRY - 1) [[unlikely]] {
    return false;
  }

  virt_addr_t cap_space_base_va = virt_addr_t::from(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * NUM_PAGE_TABLE_ENTRY * task->cap_count.num_extension);

  page_table_t* page_table = task->root_page_table;
  pte_t*        pte        = nullptr;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(cap_space_base_va, level);
    assert(pte->is_enabled());
    page_table = pte->get_next_page().as<page_table_t*>();
  }

  pte = page_table->walk(cap_space_base_va, MEGA_PAGE_TABLE_LEVEL);
  assert(pte->is_disabled());
  pte->set_next_page(page);
  pte->enable();

  ++task->cap_count.num_extension;

  return true;
}

void kill_task(task_t* task) {
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

void switch_task(task_t* task) {
  assert(task != nullptr);

  task_t* old_task = get_cls()->current_task;

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
  switch_context(&task->context, &old_task->context);
  get_cls()->current_task = old_task;
}

void suspend_task(task_t* task) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  // TODO: impl
}

void resume_task(task_t* task) {
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

task_t* lookup_tid(tid_t tid) {
  if (tid.index == 0) [[unlikely]] {
    return nullptr;
  }

  std::lock_guard<spinlock_t> lock(lookup_tid_lock);

  auto old_handler = signal(SIGSEGV, lookup_tid_signal_handler);
  if (old_handler == SIG_ERR) [[unlikely]] {
    panic("Failed to set signal handler.");
  }

  // If the task's area is not mapped, accessing it will cause a page fault, resulting in a SEGV.
  // The signal handler for SEGV is set to 'lookup_tid_signal_handler', which uses 'longjmp' to safely return control, subsequently setting 'task' to 'nullptr' in the 'else' branch of the 'setjmp' check.

  task_t* task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE) + tid.index;
  if (setjmp(jump_buffer) == 0) {
    if (task->state == task_state_t::unused) [[unlikely]] {
      task = nullptr;
    }
  } else {
    task = nullptr;
  }

  signal(SIGSEGV, old_handler);
  return task;
}
