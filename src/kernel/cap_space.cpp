#include <algorithm>
#include <cassert>
#include <iterator>
#include <mutex>

#include <kernel/align.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/task.h>

bool insert_cap_space(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space) {
  assert(task != nullptr);
  assert(cap_space != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->cap_count.num_cap_space / NUM_PAGE_TABLE_ENTRY > task->cap_count.num_extension) [[unlikely]] {
    return false;
  }

  virt_ptr<void> cap_space_base_va = make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * task->cap_count.num_cap_space);

  map_ptr<page_table_t> page_table = task->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= MEGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(cap_space_base_va, level);
    assert(pte->is_enabled());
    page_table = pte->get_next_page().as<page_table_t>();
  }

  pte = page_table->walk(cap_space_base_va, KILO_PAGE_TABLE_LEVEL);
  assert(pte->is_disabled());
  pte->set_next_page(cap_space.as<void>());
  pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
  pte->enable();

  auto end = std::rend(cap_space->slots);
  if (task->cap_count.num_cap_space == 0) [[unlikely]] {
    // The first element of cap-space is always null-cap.
    --end;
    cap_space->slots[0].cap  = make_null_cap();
    cap_space->slots[0].next = 0_map;
    cap_space->slots[0].prev = 0_map;
  }

  std::for_each(std::rbegin(cap_space->slots), end, [task](auto&& slots) {
    slots.prev       = task->free_slots;
    task->free_slots = make_map_ptr(&slots);
  });

  cap_space->meta_info.map         = cap_space;
  cap_space->meta_info.space_index = task->cap_count.num_cap_space;

  ++task->cap_count.num_cap_space;

  return true;
}

bool extend_cap_space(map_ptr<task_t> task, map_ptr<void> page) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->cap_count.num_extension == NUM_PAGE_TABLE_ENTRY - 1) [[unlikely]] {
    return false;
  }

  virt_ptr<void> cap_space_base_va = make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * NUM_PAGE_TABLE_ENTRY * task->cap_count.num_extension);

  map_ptr<page_table_t> page_table = task->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(cap_space_base_va, level);
    assert(pte->is_enabled());
    page_table = pte->get_next_page().as<page_table_t>();
  }

  pte = page_table->walk(cap_space_base_va, MEGA_PAGE_TABLE_LEVEL);
  assert(pte->is_disabled());
  pte->set_next_page(page);
  pte->enable();

  ++task->cap_count.num_extension;

  return true;
}

map_ptr<cap_slot_t> lookup_cap(map_ptr<task_t> task, uintptr_t cap_desc) {
  assert(task != nullptr);

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  uintptr_t capacity = task->cap_count.num_cap_space * std::size(static_cast<cap_space_t*>(nullptr)->slots);
  if (cap_desc >= capacity) [[unlikely]] {
    return 0_map;
  }

  size_t space_index = cap_desc / std::size(static_cast<cap_space_t*>(nullptr)->slots);
  size_t slot_index  = cap_desc % std::size(static_cast<cap_space_t*>(nullptr)->slots);

  virt_ptr<void> va = make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * space_index);

  map_ptr<page_table_t> page_table = task->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= MEGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(va, level);
    if (pte->is_disabled()) [[unlikely]] {
      return 0_map;
    }
    page_table = pte->get_next_page().as<page_table_t>();
  }

  pte = page_table->walk(va, KILO_PAGE_TABLE_LEVEL);
  if (pte->is_disabled()) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_space_t> cap_space = pte->get_next_page().as<cap_space_t>();
  return make_map_ptr(&cap_space->slots[slot_index]);
}

size_t get_cap_slot_index(map_ptr<cap_slot_t> cap_slot) {
  assert(cap_slot != nullptr);

  map_ptr<cap_space_t> cap_space = make_map_ptr(round_down(cap_slot.raw(), PAGE_SIZE));

  size_t space_index = cap_space->meta_info.space_index;
  size_t slot_index  = cap_slot.get() - cap_space->slots;

  return std::size(static_cast<cap_space_t*>(nullptr)->slots) * space_index + slot_index;
}
