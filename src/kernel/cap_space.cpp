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
      case CAP_ID:
        destroy_id_object(slot);
        break;
      default:
        panic("Unexcepted cap type.");
    }
  }

  void destroy_cap_slot(map_ptr<cap_slot_t> slot) {
    assert(slot != nullptr);
    assert(slot->is_tail());

    map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

    std::lock_guard lock(task->lock);

    if (task->state == task_state_t::unused) [[unlikely]] {
      panic("Unexpected task state.");
    }

    if (!slot->is_head()) {
      map_ptr<cap_slot_t> prev_slot = slot->prev;
      cap_type_t          prev_type = get_cap_type(prev_slot->cap);

      // prev_slot is delegated cap.
      if (prev_type == CAP_ZOMBIE) {
        prev_slot->cap = slot->cap;
      }
      // slot is created by create_object.
      else if (!is_same_object(prev_slot, slot)) {
        destroy_object(slot);
      }
    } else {
      destroy_object(slot);
    }

    slot->erase_this();
    push_free_slots(task, slot);
  }
} // namespace

map_ptr<cap_space_t> cap_slot_t::get_cap_space() const {
  return make_map_ptr(round_down(reinterpret_cast<uintptr_t>(this), PAGE_SIZE));
}

bool cap_slot_t::is_unused() const {
  return get_cap_type(cap) == CAP_NULL && prev == nullptr && next == nullptr;
}

bool cap_slot_t::is_isolated() const {
  return prev == nullptr && next == nullptr;
}

bool cap_slot_t::is_head() const {
  return prev == nullptr;
}

bool cap_slot_t::is_tail() const {
  return next == nullptr;
}

void cap_slot_t::insert_before(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);
  assert(slot->is_isolated());

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  if (this->prev != nullptr) {
    this->prev->next = slot;
    slot->prev       = this->prev;
  }

  slot->next = make_map_ptr(this);
  this->prev = slot;
}

void cap_slot_t::insert_after(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);
  assert(slot->is_head());

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  map_ptr<cap_slot_t> tail = slot;
  while (!tail->is_tail()) {
    tail = tail->next;
  }

  if (this->next != nullptr) {
    this->next->prev = tail;
    tail->next       = this->next;
  }

  this->next = slot;
  slot->prev = make_map_ptr(this);
}

map_ptr<cap_slot_t> cap_slot_t::erase_this() {
  map_ptr<task_t>& task = get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  map_ptr<cap_slot_t> result = 0_map;

  if (prev != nullptr) {
    prev->next = next;
    result     = prev;
  }

  if (next != nullptr) {
    next->prev = prev;
    result     = next;
  }

  prev = 0_map;
  next = 0_map;

  return result;
}

void cap_slot_t::replace(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);
  assert(this->is_isolated());

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  if (slot->prev != nullptr) {
    slot->prev->next = make_map_ptr(this);
    this->prev       = slot->prev;
    slot->prev       = 0_map;
  }

  if (slot->next != nullptr) {
    slot->next->prev = make_map_ptr(this);
    this->next       = slot->next;
    slot->next       = 0_map;
  }
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

  dst_slot->replace(src_slot);
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

  src_slot->cap = make_zombie_cap();
  src_slot->insert_after(dst_slot);

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
      src_slot->insert_after(dst_slot);
      break;
    case CAP_ENDPOINT:
      dst_slot = insert_cap(src_task, src_slot->cap);
      src_slot->insert_after(dst_slot);
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

  return dst_slot;
}

bool revoke_cap(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);

  cap_type_t type = get_cap_type(slot->cap);
  if (type != CAP_MEM && type != CAP_ZOMBIE) [[unlikely]] {
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  if (type == CAP_MEM) {
    map_ptr<cap_slot_t> cap_slot = slot;
    while (!cap_slot->is_tail()) {
      cap_slot = cap_slot->next;
    }
    while (cap_slot != slot) {
      map_ptr<cap_slot_t> prev_slot = cap_slot->prev;
      destroy_cap_slot(cap_slot);
      cap_slot = prev_slot;
    }
  } else {
    assert(type == CAP_ZOMBIE);

    map_ptr<cap_slot_t> cap_slot = slot->next;
    while (get_cap_type(cap_slot->cap) == CAP_ZOMBIE) {
      cap_slot = cap_slot->next;
    }

    capability_t cap = cap_slot->cap;

    while (cap_slot != slot) {
      map_ptr<cap_slot_t> prev_slot = cap_slot->prev;

      cap_slot->erase_this();
      push_free_slots(cap_slot->get_cap_space()->meta_info.task, cap_slot);

      cap_slot = prev_slot;
    }

    slot->cap = cap;
  }

  return true;
}

bool destroy_cap(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);

  cap_type_t type = get_cap_type(slot->cap);
  if (type == CAP_NULL) [[unlikely]] {
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  if (type == CAP_MEM || type == CAP_ZOMBIE) {
    if (!revoke_cap(slot)) [[unlikely]] {
      return false;
    }
  }

  if ((slot->prev == nullptr || !is_same_object(slot->prev, slot)) && (slot->next == nullptr || !is_same_object(slot->next, slot))) {
    destroy_object(slot);
    push_free_slots(slot->get_cap_space()->meta_info.task, slot);
  } else if (slot->prev != nullptr && get_cap_type(slot->prev->cap) == CAP_ZOMBIE) {
    assert(is_same_object(slot->prev, slot));
    if (!revoke_cap(slot->prev)) [[unlikely]] {
      return false;
    }
  } else {
    slot->erase_this();
    push_free_slots(slot->get_cap_space()->meta_info.task, slot);
  }

  return true;
}

bool is_same_cap(map_ptr<cap_slot_t> lhs, map_ptr<cap_slot_t> rhs) {
  assert(lhs != nullptr);
  assert(rhs != nullptr);

  if (get_cap_type(lhs->cap) != get_cap_type(rhs->cap)) {
    return false;
  }

  switch (get_cap_type(lhs->cap)) {
    case CAP_NULL:
      return false;
    case CAP_MEM:
      return false;
    case CAP_TASK:
      return lhs->cap.task.task == rhs->cap.task.task;
    case CAP_ENDPOINT:
      return lhs->cap.endpoint.endpoint == rhs->cap.endpoint.endpoint;
    case CAP_VIRT_PAGE:
      return false;
    case CAP_PAGE_TABLE:
      return false;
    case CAP_CAP_SPACE:
      return false;
    case CAP_ID:
      return lhs->cap.id.val1 == rhs->cap.id.val1 && lhs->cap.id.val2 == rhs->cap.id.val2 && lhs->cap.id.val3 == rhs->cap.id.val3;
    case CAP_ZOMBIE:
      return false;
    case CAP_UNKNOWN:
      return false;
  }

  return false;
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
  if (get_cap_type(cap_space->slots[slot_index].cap) == CAP_NULL) [[unlikely]] {
    errno = SYS_S_OK;
    return 0_map;
  }

  return make_map_ptr(&cap_space->slots[slot_index]);
}

size_t get_cap_slot_index(map_ptr<cap_slot_t> cap_slot) {
  assert(cap_slot != nullptr);

  map_ptr<cap_space_t> cap_space = cap_slot->get_cap_space();

  size_t space_index = cap_space->meta_info.space_index;
  size_t slot_index  = cap_slot.get() - cap_space->slots;

  return std::size(static_cast<cap_space_t*>(nullptr)->slots) * space_index + slot_index;
}
