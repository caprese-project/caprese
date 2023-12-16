#include <algorithm>
#include <cassert>
#include <cerrno>
#include <iterator>
#include <mutex>

#include <kernel/align.h>
#include <kernel/cap.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/log.h>
#include <kernel/task.h>
#include <libcaprese/syscall.h>

namespace {
  constexpr const char* tag = "kernel/cap_space";

  void destroy_object(map_ptr<cap_slot_t> slot) {
    assert(slot != nullptr);

    cap_type_t type = get_cap_type(slot->cap);

    switch (type) {
      case CAP_MEM:
        destroy_memory_object(slot);
        break;
      case CAP_TASK:
        destroy_task_object(slot);
        break;
      case CAP_ENDPOINT:
        destroy_endpoint_object(slot);
        break;
      case CAP_PAGE_TABLE:
        destroy_page_table_object(slot);
        break;
      case CAP_VIRT_PAGE:
        destroy_virt_page_object(slot);
        break;
      case CAP_CAP_SPACE:
        destroy_cap_space_object(slot);
        break;
      default:
        panic("Unexcepted cap type.");
    }
  }

  [[nodiscard]] void destroy_cap_slot(map_ptr<cap_slot_t> slot) {
    assert(slot != nullptr);
    assert(slot->next == nullptr);

    cap_type_t       type = get_cap_type(slot->cap);
    map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

    std::lock_guard lock(task->lock);

    if (task->state == task_state_t::unused) [[unlikely]] {
      panic("Unexpected task state.");
    }

    if (slot->prev != nullptr) {
      map_ptr<cap_slot_t> prev_slot = slot->prev;
      cap_type_t          prev_type = get_cap_type(prev_slot->cap);

      // prev_slot is delegated cap.
      if (prev_type == CAP_NULL) {
        prev_slot->cap = slot->cap;
      }
      // slot is created by create_object.
      else if (prev_type == CAP_MEM) {
        destroy_object(slot);
      }
      // prev_slot is copied cap.
      else if (prev_type == type) {
        switch (type) {
          case CAP_TASK:
            if (slot->cap.task.task != prev_slot->cap.task.task) [[unlikely]] {
              panic("Unexpected cap state");
            }
            break;
          case CAP_ENDPOINT:
            if (slot->cap.endpoint.endpoint != prev_slot->cap.endpoint.endpoint) [[unlikely]] {
              panic("Unexpected cap state");
            }
            break;
          case CAP_ID:
            if (slot->cap.id.val1 != prev_slot->cap.id.val1 || slot->cap.id.val2 != prev_slot->cap.id.val2) [[unlikely]] {
              panic("Unexpected cap state");
            }
            break;
          default:
            panic("Unexcepted cap type.");
        }
      }
      // Should not happen.
      else {
        panic("Unexpected cap state");
      }

      prev_slot->next = 0_map;
    } else {
      destroy_object(slot);
    }

    push_free_slots(task, slot);
  }
} // namespace

map_ptr<cap_space_t> cap_slot_t::get_cap_space() const {
  return make_map_ptr(round_down(reinterpret_cast<uintptr_t>(this), PAGE_SIZE));
}

bool insert_cap_space(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space) {
  assert(task != nullptr);
  assert(cap_space != nullptr);

  std::lock_guard lock(task->lock);

  if (task->cap_count.num_cap_space / NUM_PAGE_TABLE_ENTRY > task->cap_count.num_extension) [[unlikely]] {
    logd(tag, "Failed to insert cap_space. Need to extend space.");
    errno = SYS_E_ILL_STATE;
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

  cap_space->meta_info.map         = cap_space;
  cap_space->meta_info.task        = task;
  cap_space->meta_info.space_index = task->cap_count.num_cap_space;

  std::for_each(std::rbegin(cap_space->slots), end, [task](auto&& slot) { push_free_slots(task, make_map_ptr(&slot)); });

  ++task->cap_count.num_cap_space;

  return true;
}

virt_ptr<void> extend_cap_space(map_ptr<task_t> task, map_ptr<page_table_t> page) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->cap_count.num_extension == NUM_PAGE_TABLE_ENTRY - 1) [[unlikely]] {
    logd(tag, "Failed to extend cap_space. No more extension.");
    errno = SYS_E_ILL_STATE;
    return 0_virt;
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
  pte->set_flags({});
  pte->set_next_page(page.as<void>());
  pte->enable();

  ++task->cap_count.num_extension;

  return cap_space_base_va;
}

map_ptr<cap_slot_t> transfer_cap(map_ptr<task_t> dst_task, map_ptr<cap_slot_t> src_slot) {
  assert(dst_task != nullptr);
  assert(src_slot != nullptr);

  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::scoped_lock lock { src_task->lock, dst_task->lock };

  if (get_cap_type(src_slot->cap) == CAP_NULL || get_cap_type(src_slot->cap) == CAP_ZOMBIE) [[unlikely]] {
    errno = SYS_E_CAP_TYPE;
    return 0_map;
  }

  if (dst_task->state == task_state_t::unused || dst_task->state == task_state_t::killed) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  if (src_task->state == task_state_t::unused || src_task->state == task_state_t::killed) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  map_ptr<cap_slot_t> dst_slot = insert_cap(dst_task, src_slot->cap);

  if (dst_slot == nullptr) [[unlikely]] {
    return 0_map;
  }

  dst_slot->prev = src_slot->prev;
  dst_slot->next = src_slot->next;

  push_free_slots(src_task, src_slot);

  return dst_slot;
}

map_ptr<cap_slot_t> delegate_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot) {
  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::scoped_lock lock { src_task->lock, task->lock };

  if (get_cap_type(src_slot->cap) == CAP_NULL || get_cap_type(src_slot->cap) == CAP_ZOMBIE) [[unlikely]] {
    errno = SYS_E_CAP_TYPE;
    return 0_map;
  }

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  if (src_task->state == task_state_t::unused || src_task->state == task_state_t::killed) [[unlikely]] {
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  map_ptr<cap_slot_t> dst_slot = insert_cap(task, src_slot->cap);

  if (dst_slot == nullptr) [[unlikely]] {
    return 0_map;
  }

  dst_slot->prev = src_slot;
  dst_slot->next = src_slot->next;
  src_slot->cap  = make_null_cap();
  src_slot->next = dst_slot;

  return dst_slot;
}

map_ptr<cap_slot_t> copy_cap(map_ptr<cap_slot_t> src_slot) {
  assert(src_slot != nullptr);

  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(src_task->lock);

  map_ptr<cap_slot_t> dst_slot = 0_map;

  switch (get_cap_type(src_slot->cap)) {
    case CAP_NULL:
      break;
    case CAP_MEM:
      break;
    case CAP_TASK:
      dst_slot = insert_cap(src_task, src_slot->cap);
      break;
    case CAP_ENDPOINT:
      dst_slot = insert_cap(src_task, src_slot->cap);
      break;
    case CAP_PAGE_TABLE:
      break;
    case CAP_VIRT_PAGE:
      break;
    case CAP_CAP_SPACE:
      break;
    case CAP_ID:
      dst_slot = insert_cap(src_task, src_slot->cap);
      break;
    case CAP_ZOMBIE:
      break;
    case CAP_UNKNOWN:
      break;
  }

  if (dst_slot == nullptr) [[unlikely]] {
    errno = SYS_E_CAP_TYPE;
    return 0_map;
  }

  dst_slot->prev = src_slot;
  dst_slot->next = src_slot->next;
  src_slot->next = dst_slot;

  return dst_slot;
}

bool revoke_cap(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  map_ptr<cap_slot_t> cap_slot = slot;
  while (cap_slot->next != nullptr) {
    cap_slot = cap_slot->next;
  }

  while (cap_slot != slot) {
    map_ptr<cap_slot_t> prev_slot = cap_slot->prev;
    destroy_cap_slot(cap_slot);
    cap_slot = prev_slot;
  }

  return true;
}

bool destroy_cap(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);

  if (!revoke_cap(slot)) [[unlikely]] {
    return false;
  }

  destroy_cap_slot(slot);

  return true;
}

map_ptr<cap_slot_t> lookup_cap(map_ptr<task_t> task, uintptr_t cap_desc) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    logd(tag, "Failed to lookup cap. The task is not running.");
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  uintptr_t capacity = task->cap_count.num_cap_space * std::size(static_cast<cap_space_t*>(nullptr)->slots);
  if (cap_desc >= capacity) [[unlikely]] {
    logd(tag, "Failed to lookup cap. cap_desc is out of range.");
    errno = SYS_E_ILL_ARGS;
    return 0_map;
  }

  size_t space_index = cap_desc / std::size(static_cast<cap_space_t*>(nullptr)->slots);
  size_t slot_index  = cap_desc % std::size(static_cast<cap_space_t*>(nullptr)->slots);

  virt_ptr<void> va = make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE + PAGE_SIZE * space_index);

  map_ptr<page_table_t> page_table = task->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (ssize_t level = MAX_PAGE_TABLE_LEVEL; level >= static_cast<ssize_t>(KILO_PAGE_TABLE_LEVEL); --level) {
    pte = page_table->walk(va, level);
    if (pte->is_disabled()) [[unlikely]] {
      logd(tag, "Failed to lookup cap. The page is not mapped.");
      errno = SYS_E_ILL_STATE;
      return 0_map;
    }
    page_table = pte->get_next_page().as<page_table_t>();
  }

  map_ptr<cap_space_t> cap_space = page_table.as<cap_space_t>();
  return make_map_ptr(&cap_space->slots[slot_index]);
}

size_t get_cap_slot_index(map_ptr<cap_slot_t> cap_slot) {
  assert(cap_slot != nullptr);

  map_ptr<cap_space_t> cap_space = cap_slot->get_cap_space();

  size_t space_index = cap_space->meta_info.space_index;
  size_t slot_index  = cap_slot.get() - cap_space->slots;

  return std::size(static_cast<cap_space_t*>(nullptr)->slots) * space_index + slot_index;
}
