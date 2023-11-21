#include <cassert>
#include <cstring>
#include <iterator>
#include <mutex>

#include <kernel/align.h>
#include <kernel/cap.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/task.h>

map_ptr<cap_slot_t> create_memory_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, bool readable, bool writable, bool executable, size_t size, size_t alignment) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(get_cap_type(dst->cap) == CAP_NULL);

  auto& mem_cap = src->cap.memory;

  if (size == 0 || std::popcount(size) != 1) [[unlikely]] {
    return 0_map;
  }

  if (readable && !mem_cap.readable) [[unlikely]] {
    return 0_map;
  }

  if (writable && !mem_cap.writable) [[unlikely]] {
    return 0_map;
  }

  if (executable && !mem_cap.executable) [[unlikely]] {
    return 0_map;
  }

  uintptr_t base_addr = round_up(mem_cap.phys_addr + mem_cap.used_size, alignment);
  size_t    rem_size  = mem_cap.phys_addr + (1 << mem_cap.size_bit) - base_addr;

  if (rem_size < size) [[unlikely]] {
    return 0_map;
  }

  int flags = (static_cast<int>(mem_cap.device) << 0) | (static_cast<int>(readable) << 1) | (static_cast<int>(writable) << 2) | (static_cast<int>(executable) << 3);
  dst->cap  = make_memory_cap(flags, std::countr_zero(size), make_phys_ptr(base_addr));

  if (src->next != nullptr) [[unlikely]] {
    src->next->prev = dst;
    dst->next       = src->next;
  } else {
    dst->next = 0_map;
  }

  src->next = dst;
  dst->prev = src;

  mem_cap.used_size = base_addr + size - mem_cap.phys_addr;

  return dst;
}

map_ptr<cap_slot_t> create_task_object(map_ptr<cap_slot_t> dst,
                                       map_ptr<cap_slot_t> src,
                                       map_ptr<cap_slot_t> cap_space_slot,
                                       map_ptr<cap_slot_t> root_page_table_slot,
                                       map_ptr<cap_slot_t> (&cap_space_page_table_slots)[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL]) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(get_cap_type(dst->cap) == CAP_NULL);
  assert(cap_space_slot != nullptr);
  assert(get_cap_type(cap_space_slot->cap) == CAP_CAP_SPACE);
  assert(root_page_table_slot != nullptr);
  assert(get_cap_type(root_page_table_slot->cap) == CAP_PAGE_TABLE);
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 1) {
    assert(cap_space_page_table_slots[0] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[0]->cap) == CAP_PAGE_TABLE);
  }
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 2) {
    assert(cap_space_page_table_slots[1] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[1]->cap) == CAP_PAGE_TABLE);
  }
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 3) {
    assert(cap_space_page_table_slots[2] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[2]->cap) == CAP_PAGE_TABLE);
  }

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device || !mem_cap.readable || !mem_cap.writable) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_space_t>  cap_space       = cap_space_slot->cap.cap_space.space;
  map_ptr<page_table_t> root_page_table = root_page_table_slot->cap.page_table.table;
  map_ptr<page_table_t> cap_space_page_tables[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL];
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 1) {
    cap_space_page_tables[0] = cap_space_page_table_slots[0]->cap.page_table.table;
  }
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 2) {
    cap_space_page_tables[1] = cap_space_page_table_slots[1]->cap.page_table.table;
  }
  if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 3) {
    cap_space_page_tables[2] = cap_space_page_table_slots[2]->cap.page_table.table;
  }

  dst = create_memory_object(dst, src, true, true, false, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<task_t> task = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(task.get(), 0, sizeof(task_t));

  init_task(task, cap_space, root_page_table, cap_space_page_tables);

  int flags = TASK_CAP_KILLABLE | TASK_CAP_SWITCHABLE | TASK_CAP_SUSPENDABLE | TASK_CAP_RESUMABLE | TASK_CAP_REGISTER_GETTABLE | TASK_CAP_REGISTER_SETTABLE | TASK_CAP_KILL_NOTIFIABLE;
  dst->cap  = make_task_cap(flags, task);

  return dst;
}

map_ptr<cap_slot_t> create_page_table_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(get_cap_type(dst->cap) == CAP_NULL);

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device || !mem_cap.readable || !mem_cap.writable) [[unlikely]] {
    return 0_map;
  }

  dst = create_memory_object(dst, src, true, true, false, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<page_table_t> page_table = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(page_table.get(), 0, sizeof(page_table_t));

  dst->cap = make_page_table_cap(0, page_table);

  return dst;
}

map_ptr<cap_slot_t> create_virt_page_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, bool readable, bool writable, bool executable, uint64_t level) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(get_cap_type(dst->cap) == CAP_NULL);

  if (level >= MAX_PAGE_TABLE_LEVEL) [[unlikely]] {
    return 0_map;
  }

  size_t page_size = get_page_size(level);
  dst              = create_memory_object(dst, src, readable, writable, executable, page_size, page_size);
  if (dst == nullptr) [[unlikely]] {
    return 0_map;
  }

  int flags = (static_cast<int>(readable) << 0) | (static_cast<int>(writable) << 1) | (static_cast<int>(executable) << 2);
  dst->cap  = make_virt_page_cap(flags, level, dst->cap.memory.phys_addr);

  return dst;
}

map_ptr<cap_slot_t> create_cap_space_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(get_cap_type(dst->cap) == CAP_NULL);

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device || !mem_cap.readable || !mem_cap.writable) [[unlikely]] {
    return 0_map;
  }

  dst = create_memory_object(dst, src, true, true, false, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_space_t> cap_space = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(cap_space.get(), 0, sizeof(cap_space_t));

  dst->cap = make_cap_space_cap(cap_space);

  return dst;
}

map_ptr<cap_slot_t> create_object(map_ptr<task_t> task, map_ptr<cap_slot_t> cap_slot, cap_type_t type, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4) {
  assert(task == get_cls()->current_task);
  assert(cap_slot != nullptr);

  if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
    return 0_map;
  }

  std::lock_guard<recursive_spinlock_t> lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  if (task->free_slots == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_slot_t> free_slots = task->free_slots->prev;

  map_ptr<cap_slot_t> result = 0_map;

  switch (type) {
    case CAP_MEM:
      result = create_memory_object(task->free_slots, cap_slot, arg0, arg1, arg2, arg3, arg4);
      break;
    case CAP_TASK: {
      map_ptr<cap_slot_t> cap_space_slot = lookup_cap(task, arg0);
      if (cap_space_slot == nullptr || get_cap_type(cap_space_slot->cap) != CAP_CAP_SPACE) [[unlikely]] {
        return 0_map;
      }

      map_ptr<cap_slot_t> root_page_table_slot = lookup_cap(task, arg1);
      if (root_page_table_slot == nullptr || get_cap_type(root_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
        return 0_map;
      }

      map_ptr<cap_slot_t> cap_space_page_table_slots[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL];
      static_assert(std::size(cap_space_page_table_slots) <= 3);

      if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 1) {
        cap_space_page_table_slots[0] = lookup_cap(task, arg2);
        if (cap_space_page_table_slots[0] == nullptr || get_cap_type(cap_space_page_table_slots[0]->cap) != CAP_PAGE_TABLE) [[unlikely]] {
          return 0_map;
        }
      }

      if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 2) {
        cap_space_page_table_slots[1] = lookup_cap(task, arg3);
        if (cap_space_page_table_slots[1] == nullptr || get_cap_type(cap_space_page_table_slots[1]->cap) != CAP_PAGE_TABLE) [[unlikely]] {
          return 0_map;
        }
      }

      if constexpr (NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL >= 3) {
        cap_space_page_table_slots[2] = lookup_cap(task, arg4);
        if (cap_space_page_table_slots[2] == nullptr || get_cap_type(cap_space_page_table_slots[2]->cap) != CAP_PAGE_TABLE) [[unlikely]] {
          return 0_map;
        }
      }

      result = create_task_object(task->free_slots, cap_slot, cap_space_slot, root_page_table_slot, cap_space_page_table_slots);
      break;
    }
    case CAP_ENDPOINT:
      break;
    case CAP_PAGE_TABLE:
      result = create_page_table_object(task->free_slots, cap_slot);
      break;
    case CAP_VIRT_PAGE:
      result = create_virt_page_object(task->free_slots, cap_slot, arg0, arg1, arg2, arg3);
      break;
    case CAP_CAP_SPACE:
      result = create_cap_space_object(task->free_slots, cap_slot);
      break;
    default:
      break;
  }

  if (result == nullptr) [[unlikely]] {
    return 0_map;
  }

  task->free_slots = free_slots;

  return result;
}

bool map_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    return false;
  }

  if (child_page_table_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(child_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    return false;
  }

  auto& page_table_cap       = page_table_slot->cap.page_table;
  auto& child_page_table_cap = child_page_table_slot->cap.page_table;

  if (page_table_cap.level == KILO_PAGE_TABLE_LEVEL) [[unlikely]] {
    return false;
  }

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_enabled()) [[unlikely]] {
    return false;
  }

  if (child_page_table_cap.mapped) [[unlikely]] {
    return false;
  }

  pte.set_flags({});
  pte.set_next_page(child_page_table_cap.table.as<void>());
  pte.enable();

  child_page_table_cap.mapped         = true;
  child_page_table_cap.level          = page_table_cap.level - 1;
  child_page_table_cap.virt_addr_base = page_table_cap.virt_addr_base + get_page_size(page_table_cap.level) * index;

  return true;
}

bool unmap_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    return false;
  }

  if (child_page_table_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(child_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    return false;
  }

  auto& page_table_cap       = page_table_slot->cap.page_table;
  auto& child_page_table_cap = child_page_table_slot->cap.page_table;

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_disabled()) [[unlikely]] {
    return false;
  }

  if (!child_page_table_cap.mapped) [[unlikely]] {
    return false;
  }

  if (pte.get_next_page() != child_page_table_cap.table.as<void>()) [[unlikely]] {
    return false;
  }

  pte.disable();

  child_page_table_cap.mapped = false;

  return true;
}

bool map_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    return false;
  }

  auto& page_table_cap = page_table_slot->cap.page_table;
  auto& virt_page_cap  = virt_page_slot->cap.virt_page;

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_enabled()) [[unlikely]] {
    return false;
  }

  if (virt_page_cap.mapped) [[unlikely]] {
    return false;
  }

  if (virt_page_cap.level != page_table_cap.level) [[unlikely]] {
    return false;
  }

  pte.set_flags({
      .readable   = readable,
      .writable   = writable,
      .executable = executable,
      .user       = 0,
      .global     = 0,
  });
  pte.set_next_page(make_phys_ptr(virt_page_cap.phys_addr));
  pte.enable();

  virt_page_cap.mapped     = true;
  virt_page_cap.readable   = readable;
  virt_page_cap.writable   = writable;
  virt_page_cap.executable = executable;
  virt_page_cap.index      = index;
  virt_page_cap.address    = virt_ptr<void>::from(page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index);

  return true;
}

bool unmap_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    return false;
  }

  auto& page_table_cap = page_table_slot->cap.page_table;
  auto& virt_page_cap  = virt_page_slot->cap.virt_page;

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_disabled()) [[unlikely]] {
    return false;
  }

  if (!virt_page_cap.mapped) [[unlikely]] {
    return false;
  }

  map_ptr<void> map_ptr = make_phys_ptr(virt_page_cap.phys_addr);

  if (pte.get_next_page() != map_ptr) [[unlikely]] {
    return false;
  }

  pte.disable();

  virt_page_cap.mapped = false;

  return true;
}

bool remap_virt_page_cap(
    map_ptr<cap_slot_t> new_page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable, map_ptr<cap_slot_t> old_page_table_slot) {
  assert(new_page_table_slot != nullptr);
  assert(get_cap_type(new_page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    return false;
  }

  if (old_page_table_slot == nullptr) [[unlikely]] {
    return false;
  }

  if (get_cap_type(old_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    return false;
  }

  auto& new_page_table_cap = new_page_table_slot->cap.page_table;
  auto& virt_page_cap      = virt_page_slot->cap.virt_page;
  auto& old_page_table_cap = old_page_table_slot->cap.page_table;

  if (!virt_page_cap.mapped) [[unlikely]] {
    return false;
  }

  if (new_page_table_cap.level != old_page_table_cap.level) [[unlikely]] {
    return false;
  }

  if (new_page_table_cap.level != virt_page_cap.level) [[unlikely]] {
    return false;
  }

  pte_t& new_pte = new_page_table_cap.table->entries[index];
  if (new_pte.is_enabled()) [[unlikely]] {
    return false;
  }

  pte_t& old_pte = old_page_table_cap.table->entries[virt_page_cap.index];
  if (old_pte.is_disabled()) [[unlikely]] {
    return false;
  }

  map_ptr<void> map_ptr = make_phys_ptr(virt_page_cap.phys_addr);

  if (old_pte.get_next_page() != map_ptr) [[unlikely]] {
    return false;
  }

  old_pte.disable();

  new_pte.set_flags({
      .readable   = readable,
      .writable   = writable,
      .executable = executable,
      .user       = true,
      .global     = false,
  });
  new_pte.set_next_page(map_ptr);
  new_pte.enable();

  virt_page_cap.readable   = readable;
  virt_page_cap.writable   = writable;
  virt_page_cap.executable = executable;
  virt_page_cap.index      = index;
  virt_page_cap.address    = virt_ptr<void>::from(new_page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index);

  return true;
}
