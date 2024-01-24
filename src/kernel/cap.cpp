#include <cassert>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <limits>
#include <mutex>

#include <kernel/align.h>
#include <kernel/cap.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/ipc.h>
#include <kernel/lock.h>
#include <kernel/log.h>
#include <kernel/task.h>
#include <libcaprese/syscall.h>

namespace {
  constexpr const char* tag = "kernel/cap";

  spinlock_t id_cap_lock;
  uint64_t   next_id[3];
} // namespace

capability_t make_unique_id_cap() {
  std::lock_guard lock(id_cap_lock);

  uint64_t val1 = next_id[0];
  uint64_t val2 = next_id[1];
  uint64_t val3 = next_id[2];

  if (next_id[2] == std::numeric_limits<uint64_t>::max()) [[unlikely]] {
    if (next_id[1] == std::numeric_limits<uint64_t>::max()) [[unlikely]] {
      ++next_id[0];
      next_id[1] = 0;
    } else {
      ++next_id[1];
    }
    next_id[2] = 0;
  } else {
    ++next_id[2];
  }

  return make_id_cap(val1, val2, val3);
}

map_ptr<cap_slot_t> create_memory_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, size_t size, size_t alignment) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());

  auto& mem_cap = src->cap.memory;

  if (size == 0) [[unlikely]] {
    logd(tag, "Failed to create memory object. Memory size must be greater than 0.");
    errno = SYS_E_ILL_ARGS;
    return 0_map;
  }

  uintptr_t base_addr = round_up(mem_cap.phys_addr + mem_cap.used_size, alignment);
  size_t    rem_size  = mem_cap.phys_addr + mem_cap.size - base_addr;

  if (rem_size < size) [[unlikely]] {
    logd(tag, "Failed to create memory object. Not enough memory.");
    errno = SYS_E_CAP_STATE;
    return 0_map;
  }

  dst->cap          = make_memory_cap(mem_cap.device, size, make_phys_ptr(base_addr));
  mem_cap.used_size = base_addr + size - mem_cap.phys_addr;
  src->insert_after(dst);

  return dst;
}

map_ptr<cap_slot_t> create_task_object(map_ptr<cap_slot_t> dst,
                                       map_ptr<cap_slot_t> src,
                                       map_ptr<cap_slot_t> cap_space_slot,
                                       map_ptr<cap_slot_t> root_page_table_slot,
                                       map_ptr<cap_slot_t> (&cap_space_page_table_slots)[NUM_INTER_PAGE_TABLE + 1]) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());
  assert(cap_space_slot != nullptr);
  assert(get_cap_type(cap_space_slot->cap) == CAP_CAP_SPACE);
  assert(root_page_table_slot != nullptr);
  assert(get_cap_type(root_page_table_slot->cap) == CAP_PAGE_TABLE);
  assert(root_page_table_slot->cap.page_table.mapped == false);
  if constexpr (NUM_INTER_PAGE_TABLE + 1 >= 1) {
    assert(cap_space_page_table_slots[0] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[0]->cap) == CAP_PAGE_TABLE);
    assert(cap_space_page_table_slots[0]->cap.page_table.mapped == false);
  }
  if constexpr (NUM_INTER_PAGE_TABLE + 1 >= 2) {
    assert(cap_space_page_table_slots[1] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[1]->cap) == CAP_PAGE_TABLE);
    assert(cap_space_page_table_slots[1]->cap.page_table.mapped == false);
  }
  if constexpr (NUM_INTER_PAGE_TABLE + 1 >= 3) {
    assert(cap_space_page_table_slots[2] != nullptr);
    assert(get_cap_type(cap_space_page_table_slots[2]->cap) == CAP_PAGE_TABLE);
    assert(cap_space_page_table_slots[2]->cap.page_table.mapped == false);
  }

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device) [[unlikely]] {
    logd(tag, "Failed to create task object. Memory must not be device.");
    errno = SYS_E_CAP_STATE;
    return 0_map;
  }

  map_ptr<cap_space_t>  cap_space       = cap_space_slot->cap.cap_space.space;
  map_ptr<page_table_t> root_page_table = root_page_table_slot->cap.page_table.table;
  map_ptr<page_table_t> cap_space_page_tables[NUM_INTER_PAGE_TABLE + 1];
  for (size_t i = 0; i < std::size(cap_space_page_table_slots); ++i) {
    cap_space_page_tables[i] = cap_space_page_table_slots[i]->cap.page_table.table;
  }

  dst = create_memory_object(dst, src, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    logd(tag, "Failed to create task object. This is due to the failure to create a memory object.");
    return 0_map;
  }

  map_ptr<task_t> task = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(task.get(), 0, sizeof(task_t));

  init_task(task, cap_space, root_page_table, cap_space_page_tables);

  int flags = TASK_CAP_KILLABLE | TASK_CAP_SWITCHABLE | TASK_CAP_SUSPENDABLE | TASK_CAP_RESUMABLE | TASK_CAP_REGISTER_GETTABLE | TASK_CAP_REGISTER_SETTABLE | TASK_CAP_KILL_NOTIFIABLE;
  dst->cap  = make_task_cap(flags, task);

  root_page_table_slot->cap.page_table.level          = MAX_PAGE_TABLE_LEVEL;
  root_page_table_slot->cap.page_table.mapped         = true;
  root_page_table_slot->cap.page_table.virt_addr_base = 0;

  for (size_t i = 0; i < std::size(cap_space_page_table_slots); ++i) {
    cap_space_page_table_slots[i]->cap.page_table.level          = i;
    cap_space_page_table_slots[i]->cap.page_table.mapped         = true;
    cap_space_page_table_slots[i]->cap.page_table.virt_addr_base = CONFIG_CAPABILITY_SPACE_BASE;
  }

  return dst;
}

map_ptr<cap_slot_t> create_endpoint_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device) [[unlikely]] {
    logd(tag, "Failed to create endpoint object. Memory must not be device.");
    errno = SYS_E_CAP_STATE;
    return 0_map;
  }

  dst = create_memory_object(dst, src, get_cap_size(CAP_ENDPOINT), get_cap_size(CAP_ENDPOINT));
  if (dst == nullptr) [[unlikely]] {
    logd(tag, "Failed to create endpoint object. This is due to the failure to create a memory object.");
    return 0_map;
  }

  map_ptr<endpoint_t> endpoint = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(endpoint.get(), 0, sizeof(endpoint_t));

  dst->cap = make_endpoint_cap(endpoint);

  return dst;
}

map_ptr<cap_slot_t> create_page_table_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device) [[unlikely]] {
    logd(tag, "Failed to create page table object. Memory must not be device.");
    errno = SYS_E_CAP_STATE;
    return 0_map;
  }

  dst = create_memory_object(dst, src, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    logd(tag, "Failed to create page table object. This is due to the failure to create a memory object.");
    return 0_map;
  }

  map_ptr<page_table_t> page_table = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(page_table.get(), 0, sizeof(page_table_t));

  dst->cap = make_page_table_cap(page_table, false, 0, 0_virt, 0_map);

  return dst;
}

map_ptr<cap_slot_t> create_virt_page_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, bool readable, bool writable, bool executable, uint64_t level) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());

  if (level > MAX_PAGE_TABLE_LEVEL) [[unlikely]] {
    logd(tag, "Failed to create virt page object. Level must be less than or equal to %llu.", MAX_PAGE_TABLE_LEVEL);
    errno = SYS_E_ILL_ARGS;
    return 0_map;
  }

  size_t page_size = get_page_size(level);
  dst              = create_memory_object(dst, src, page_size, page_size);
  if (dst == nullptr) [[unlikely]] {
    logd(tag, "Failed to create virt page object. This is due to the failure to create a memory object.");
    return 0_map;
  }

  dst->cap = make_virt_page_cap(dst->cap.memory.device, readable, writable, executable, false, level, make_phys_ptr(dst->cap.memory.phys_addr), 0_virt, 0_map);

  return dst;
}

map_ptr<cap_slot_t> create_cap_space_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src) {
  assert(src != nullptr);
  assert(get_cap_type(src->cap) == CAP_MEM);
  assert(dst != nullptr);
  assert(dst->is_unused());

  auto& mem_cap = src->cap.memory;
  if (mem_cap.device) [[unlikely]] {
    logd(tag, "Failed to create cap space object. Memory must not be device.");
    errno = SYS_E_CAP_STATE;
    return 0_map;
  }

  dst = create_memory_object(dst, src, PAGE_SIZE, PAGE_SIZE);
  if (dst == nullptr) [[unlikely]] {
    logd(tag, "Failed to create cap space object. This is due to the failure to create a memory object.");
    return 0_map;
  }

  map_ptr<cap_space_t> cap_space = make_phys_ptr(dst->cap.memory.phys_addr);
  memset(cap_space.get(), 0, sizeof(cap_space_t));

  dst->cap = make_cap_space_cap(cap_space, false);

  return dst;
}

map_ptr<cap_slot_t> create_id_object(map_ptr<cap_slot_t> dst) {
  assert(dst != nullptr);
  assert(dst->is_unused());

  dst->cap = make_unique_id_cap();

  return dst;
}

map_ptr<cap_slot_t> create_object(map_ptr<task_t> task, map_ptr<cap_slot_t> cap_slot, cap_type_t type, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4) {
  assert(task == get_cls()->current_task);
  assert(cap_slot != nullptr);

  if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
    logd(tag, "Failed to create object. cap_slot must be memory cap.");
    errno = SYS_E_CAP_TYPE;
    return 0_map;
  }

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    logd(tag, "Failed to create object. The task is not running.");
    errno = SYS_E_ILL_STATE;
    return 0_map;
  }

  map_ptr<cap_slot_t> slot = pop_free_slots(task);
  if (slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to create object. No more free slots.");
    errno = SYS_E_OUT_OF_CAP_SPACE;
    return 0_map;
  }

  map_ptr<cap_slot_t> result = 0_map;

  switch (type) {
    case CAP_NULL:
      logw(tag, "Cannot create a null object.");
      break;
    case CAP_MEM:
      result = create_memory_object(slot, cap_slot, arg0, arg1);
      break;
    case CAP_TASK: {
      map_ptr<cap_slot_t> cap_space_slot = lookup_cap(task, arg0);
      if (cap_space_slot == nullptr) [[unlikely]] {
        logd(tag, "Failed to create task object. Failed to lookup cap slot (%llu).", arg0);
        errno = SYS_E_ILL_ARGS;
        break;
      }
      if (get_cap_type(cap_space_slot->cap) != CAP_CAP_SPACE) [[unlikely]] {
        logd(tag, "Failed to create task object. cap slot (%llu) must be cap space cap.", arg0);
        errno = SYS_E_CAP_TYPE;
        break;
      }

      map_ptr<cap_slot_t> root_page_table_slot = lookup_cap(task, arg1);
      if (root_page_table_slot == nullptr) [[unlikely]] {
        logd(tag, "Failed to create task object. Failed to lookup cap slot (%llu).", arg1);
        errno = SYS_E_ILL_ARGS;
        break;
      }
      if (get_cap_type(root_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
        logd(tag, "Failed to create task object. cap slot (%llu) must be page table cap.", arg1);
        errno = SYS_E_CAP_TYPE;
        break;
      }
      if (root_page_table_slot->cap.page_table.mapped) [[unlikely]] {
        logd(tag, "Failed to create task object. cap slot (%llu) must not be mapped.", arg1);
        errno = SYS_E_CAP_STATE;
        break;
      }

      map_ptr<cap_slot_t> cap_space_page_table_slots[NUM_INTER_PAGE_TABLE + 1];
      static_assert(std::size(cap_space_page_table_slots) <= 3);

      const uintptr_t cap_descs[] { arg2, arg3, arg4 };
      for (size_t i = 0; i < std::size(cap_space_page_table_slots); ++i) {
        cap_space_page_table_slots[i] = lookup_cap(task, cap_descs[i]);
        if (cap_space_page_table_slots[i] == nullptr) [[unlikely]] {
          logd(tag, "Failed to create task object. Failed to lookup cap slot (%llu).", cap_descs[i]);
          errno = SYS_E_ILL_ARGS;
          break;
        }
        if (get_cap_type(cap_space_page_table_slots[i]->cap) != CAP_PAGE_TABLE) [[unlikely]] {
          logd(tag, "Failed to create task object. cap slot (%llu) must be page table cap.", cap_descs[i]);
          errno = SYS_E_CAP_TYPE;
          break;
        }
        if (cap_space_page_table_slots[i]->cap.page_table.mapped) [[unlikely]] {
          logd(tag, "Failed to create task object. cap slot (%llu) must not be mapped.", cap_descs[i]);
          errno = SYS_E_CAP_STATE;
          break;
        }
      }

      result = create_task_object(slot, cap_slot, cap_space_slot, root_page_table_slot, cap_space_page_table_slots);
      break;
    }
    case CAP_ENDPOINT:
      result = create_endpoint_object(slot, cap_slot);
      break;
    case CAP_PAGE_TABLE:
      result = create_page_table_object(slot, cap_slot);
      break;
    case CAP_VIRT_PAGE:
      result = create_virt_page_object(slot, cap_slot, arg0, arg1, arg2, arg3);
      break;
    case CAP_CAP_SPACE:
      result = create_cap_space_object(slot, cap_slot);
      break;
    case CAP_ID:
      logw(tag, "The id object cannot be created from the memory object. Use sys_id_cap_create.");
      break;
    case CAP_ZOMBIE:
      logw(tag, "Cannot create a zombie object.");
      break;
    case CAP_UNKNOWN:
      logw(tag, "Cannot create an unknown object.");
      break;
  }

  if (result == nullptr) [[unlikely]] {
    push_free_slots(task, slot);
    return 0_map;
  }

  return result;
}

bool is_same_object(map_ptr<cap_slot_t> lhs, map_ptr<cap_slot_t> rhs) {
  assert(lhs != nullptr);
  assert(rhs != nullptr);

  while (get_cap_type(lhs->cap) == CAP_ZOMBIE) {
    lhs = lhs->next;
  }

  while (get_cap_type(rhs->cap) == CAP_ZOMBIE) {
    rhs = rhs->next;
  }

  // If it's not copyable, it should point to the same address.
  if (lhs == rhs) {
    return true;
  }

  cap_type_t lhs_type = get_cap_type(lhs->cap);
  cap_type_t rhs_type = get_cap_type(rhs->cap);

  if (lhs_type != rhs_type) {
    return false;
  }

  switch (lhs_type) {
    case CAP_TASK:
      return lhs->cap.task.task == rhs->cap.task.task;
    case CAP_ENDPOINT:
      return lhs->cap.endpoint.endpoint == rhs->cap.endpoint.endpoint;
    case CAP_ID:
      return lhs->cap.id.val1 == rhs->cap.id.val1 && lhs->cap.id.val2 == rhs->cap.id.val2 && lhs->cap.id.val3 == rhs->cap.id.val3;
    default:
      return false;
  }
}

void destroy_memory_object(map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail());
  assert(get_cap_type(slot->cap) == CAP_MEM);
  slot->cap.memory.used_size = 0;
}

void destroy_task_object(map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_TASK);
  kill_task(slot->cap.task.task, 0);
}

void destroy_endpoint_object(map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_ENDPOINT);
  ipc_cancel(slot->cap.endpoint.endpoint);
}

void destroy_page_table_object(map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_PAGE_TABLE);

  map_ptr<page_table_t> parent_table = slot->cap.page_table.parent_table;
  if (parent_table != nullptr) {
    map_ptr<pte_t> pte = parent_table->walk(make_virt_ptr(slot->cap.page_table.virt_addr_base), slot->cap.page_table.level + 1);
    assert(pte->is_enabled());
    assert(pte->get_next_page() == slot->cap.page_table.table.as<void>());
    pte->disable();
  }
}

void destroy_virt_page_object(map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_VIRT_PAGE);

  map_ptr<page_table_t> parent_table = slot->cap.virt_page.parent_table;
  if (parent_table != nullptr) {
    map_ptr<pte_t> pte = parent_table->walk(slot->cap.virt_page.address, slot->cap.virt_page.level);
    assert(pte->is_enabled());
    assert(pte->get_next_page() == phys_ptr<void>::from(slot->cap.virt_page.phys_addr).as_map());
    pte->disable();
  }
}

void destroy_cap_space_object([[maybe_unused]] map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_CAP_SPACE);

  // TODO: impl
}

void destroy_id_object([[maybe_unused]] map_ptr<cap_slot_t> slot) {
  assert(slot->is_tail() || !is_same_object(slot, slot->next));
  assert(get_cap_type(slot->cap) == CAP_ID);

  // Do nothing.
}

bool map_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    logd(tag, "Failed to map page table cap. index must be less than %llu. (index=%llu)", NUM_PAGE_TABLE_ENTRY, index);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (child_page_table_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to map page table cap. child_page_table_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(child_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    logd(tag, "Failed to map page table cap. child_page_table_slot must be page table cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  auto& page_table_cap       = page_table_slot->cap.page_table;
  auto& child_page_table_cap = child_page_table_slot->cap.page_table;

  uintptr_t va = page_table_cap.virt_addr_base + get_page_size(page_table_cap.level) * index;
  if (va >= CONFIG_KERNEL_SPACE_BASE) [[unlikely]] {
    logd(tag, "Failed to map page table cap. Virtual address must be less than %p. (index=%llu, addr=%p, level=%d)", CONFIG_KERNEL_SPACE_BASE, index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (page_table_cap.level == KILO_PAGE_TABLE_LEVEL) [[unlikely]] {
    logd(tag, "Failed to map page table cap. Cannot map page table cap to kilo page table. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_enabled()) [[unlikely]] {
    logd(tag, "Failed to map page table cap. Page table entry must be disabled. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (child_page_table_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to map page table cap. Page table cap must not be mapped. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  pte.set_flags({});
  pte.set_next_page(child_page_table_cap.table.as<void>());
  pte.enable();

  child_page_table_cap.mapped         = true;
  child_page_table_cap.level          = page_table_cap.level - 1;
  child_page_table_cap.virt_addr_base = va;
  child_page_table_cap.parent_table   = page_table_cap.table;

  return true;
}

bool unmap_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. index must be less than %llu. (index=%llu)", NUM_PAGE_TABLE_ENTRY, index);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (child_page_table_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. child_page_table_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(child_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. child_page_table_slot must be page table cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  auto& page_table_cap       = page_table_slot->cap.page_table;
  auto& child_page_table_cap = child_page_table_slot->cap.page_table;

  uintptr_t va = page_table_cap.virt_addr_base + get_page_size(page_table_cap.level) * index;
  if (va >= CONFIG_KERNEL_SPACE_BASE) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. Virtual address must be less than %p. (index=%llu, addr=%p, level=%d)", CONFIG_KERNEL_SPACE_BASE, index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_disabled()) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. Page table entry must be enabled. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (!child_page_table_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. Page table cap must be mapped. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (pte.get_next_page() != child_page_table_cap.table.as<void>()) [[unlikely]] {
    logd(tag, "Failed to unmap page table cap. child_page_table_cap is not mapped to the page table entry. (index=%llu, addr=%p, level=%d)", index, va, (int)page_table_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  pte.disable();

  child_page_table_cap.mapped       = false;
  child_page_table_cap.parent_table = 0_map;

  return true;
}

bool map_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    logd(tag, "Failed to map virt page. index must be less than %llu. (index=%llu)", NUM_PAGE_TABLE_ENTRY, index);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to map virt page. virt_page_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    logd(tag, "Failed to map virt page. virt_page_slot must be virt page cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  auto& page_table_cap = page_table_slot->cap.page_table;
  auto& virt_page_cap  = virt_page_slot->cap.virt_page;

  uintptr_t va = page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index;
  if (va >= CONFIG_KERNEL_SPACE_BASE) [[unlikely]] {
    logd(tag, "Failed to map virt page. Virtual address must be less than %p. (index=%llu, addr=%p, level=%d)", CONFIG_KERNEL_SPACE_BASE, index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_enabled()) [[unlikely]] {
    logd(tag, "Failed to map virt page. Page table entry must be disabled. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (virt_page_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to map virt page. Virt page cap must not be mapped. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (virt_page_cap.level != page_table_cap.level) [[unlikely]] {
    logd(tag, "Failed to map virt page. Virt page cap must have the same level as page table cap. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (!virt_page_cap.device) {
    memset(phys_ptr<void>::from(virt_page_cap.phys_addr).as_map().get(), 0, get_page_size(virt_page_cap.level));
  }

  pte.set_flags({
      .readable   = readable,
      .writable   = writable,
      .executable = executable,
      .user       = true,
      .global     = false,
  });
  pte.set_next_page(make_phys_ptr(virt_page_cap.phys_addr));
  pte.enable();

  virt_page_cap.mapped       = true;
  virt_page_cap.readable     = readable;
  virt_page_cap.writable     = writable;
  virt_page_cap.executable   = executable;
  virt_page_cap.index        = index;
  virt_page_cap.address      = virt_ptr<void>::from(va);
  virt_page_cap.parent_table = page_table_cap.table;

  return true;
}

bool unmap_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot) {
  assert(page_table_slot != nullptr);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. index must be less than %llu. (index=%llu)", NUM_PAGE_TABLE_ENTRY, index);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. virt_page_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. virt_page_slot must be virt page cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  auto& page_table_cap = page_table_slot->cap.page_table;
  auto& virt_page_cap  = virt_page_slot->cap.virt_page;

  uintptr_t va = page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index;
  if (va >= CONFIG_KERNEL_SPACE_BASE) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. Virtual address must be less than %p. (index=%llu, addr=%p, level=%d)", CONFIG_KERNEL_SPACE_BASE, index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  pte_t& pte = page_table_cap.table->entries[index];
  if (pte.is_disabled()) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. Page table entry must be enabled. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (!virt_page_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. Virt page cap must be mapped. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  map_ptr<void> map_ptr = make_phys_ptr(virt_page_cap.phys_addr);

  if (pte.get_next_page() != map_ptr) [[unlikely]] {
    logd(tag, "Failed to unmap virt page. virt_page_cap is not mapped to the page table entry. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  pte.disable();

  virt_page_cap.mapped       = false;
  virt_page_cap.parent_table = 0_map;

  return true;
}

bool remap_virt_page_cap(
    map_ptr<cap_slot_t> new_page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable, map_ptr<cap_slot_t> old_page_table_slot) {
  assert(new_page_table_slot != nullptr);
  assert(get_cap_type(new_page_table_slot->cap) == CAP_PAGE_TABLE);

  if (index >= NUM_PAGE_TABLE_ENTRY) [[unlikely]] {
    logd(tag, "Failed to remap virt page. index must be less than %llu. (index=%llu)", NUM_PAGE_TABLE_ENTRY, index);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (virt_page_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to remap virt page. virt_page_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(virt_page_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    logd(tag, "Failed to remap virt page. virt_page_slot must be virt page cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  if (old_page_table_slot == nullptr) [[unlikely]] {
    logd(tag, "Failed to remap virt page. old_page_table_slot must not be null.");
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (get_cap_type(old_page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    logd(tag, "Failed to remap virt page. old_page_table_slot must be page table cap.");
    errno = SYS_E_CAP_TYPE;
    return false;
  }

  auto& new_page_table_cap = new_page_table_slot->cap.page_table;
  auto& virt_page_cap      = virt_page_slot->cap.virt_page;
  auto& old_page_table_cap = old_page_table_slot->cap.page_table;

  uintptr_t va = new_page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index;
  if (va >= CONFIG_KERNEL_SPACE_BASE) [[unlikely]] {
    logd(tag, "Failed to remap virt page. Virtual address must be less than %p. (index=%llu, addr=%p, level=%d)", CONFIG_KERNEL_SPACE_BASE, index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_ARGS;
    return false;
  }

  if (!virt_page_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to remap virt page. Virt page cap must be mapped. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (new_page_table_cap.level != old_page_table_cap.level) [[unlikely]] {
    logd(tag, "Failed to remap virt page. new_page_table_cap and old_page_table_cap must have the same level. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (new_page_table_cap.level != virt_page_cap.level) [[unlikely]] {
    logd(tag, "Failed to remap virt page. new_page_table_cap and virt_page_cap must have the same level. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_CAP_STATE;
    return false;
  }

  pte_t& new_pte = new_page_table_cap.table->entries[index];
  if (new_pte.is_enabled()) [[unlikely]] {
    logd(tag, "Failed to remap virt page. New page table entry must be disabled. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  pte_t& old_pte = old_page_table_cap.table->entries[virt_page_cap.index];
  if (old_pte.is_disabled()) [[unlikely]] {
    logd(tag, "Failed to remap virt page. Old page table entry must be enabled. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
    return false;
  }

  map_ptr<void> map_ptr = make_phys_ptr(virt_page_cap.phys_addr);

  if (old_pte.get_next_page() != map_ptr) [[unlikely]] {
    logd(tag, "Failed to remap virt page. virt_page_cap is not mapped to the page table entry. (index=%llu, addr=%p, level=%d)", index, va, (int)virt_page_cap.level);
    errno = SYS_E_ILL_STATE;
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

  virt_page_cap.readable     = readable;
  virt_page_cap.writable     = writable;
  virt_page_cap.executable   = executable;
  virt_page_cap.index        = index;
  virt_page_cap.address      = virt_ptr<void>::from(new_page_table_cap.virt_addr_base + get_page_size(virt_page_cap.level) * index);
  virt_page_cap.parent_table = new_page_table_cap.table;

  return true;
}

bool insert_cap_space(map_ptr<cap_slot_t> task_slot, map_ptr<cap_slot_t> cap_space_slot) {
  assert(task_slot != nullptr);
  assert(cap_space_slot != nullptr);
  assert(get_cap_type(task_slot->cap) == CAP_TASK);
  assert(get_cap_type(cap_space_slot->cap) == CAP_CAP_SPACE);

  auto& task_cap      = task_slot->cap.task;
  auto& cap_space_cap = cap_space_slot->cap.cap_space;

  if (task_cap.task->state == task_state_t::unused || task_cap.task->state == task_state_t::killed) [[unlikely]] {
    logd(tag, "Failed to insert cap space. The task is not running.");
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (cap_space_cap.used) [[unlikely]] {
    logd(tag, "Failed to insert cap space. Cap space cap must not be used.");
    errno = SYS_E_CAP_STATE;
    return false;
  }

  if (!insert_cap_space(task_cap.task, cap_space_cap.space)) [[unlikely]] {
    return false;
  }

  cap_space_cap.used = true;

  return true;
}

bool extend_cap_space(map_ptr<cap_slot_t> task_slot, map_ptr<cap_slot_t> page_table_slot) {
  assert(task_slot != nullptr);
  assert(page_table_slot != nullptr);
  assert(get_cap_type(task_slot->cap) == CAP_TASK);
  assert(get_cap_type(page_table_slot->cap) == CAP_PAGE_TABLE);

  auto& task_cap       = task_slot->cap.task;
  auto& page_table_cap = page_table_slot->cap.page_table;

  if (task_cap.task->state == task_state_t::unused || task_cap.task->state == task_state_t::killed) [[unlikely]] {
    logd(tag, "Failed to extend cap space. The task is not running.");
    errno = SYS_E_ILL_STATE;
    return false;
  }

  if (page_table_cap.mapped) [[unlikely]] {
    logd(tag, "Failed to extend cap space. Page table cap must not be mapped.");
    errno = SYS_E_CAP_STATE;
    return false;
  }

  virt_ptr<void> va = extend_cap_space(task_cap.task, page_table_cap.table);

  if (va == nullptr) [[unlikely]] {
    return false;
  }

  map_ptr<page_table_t> page_table = task_cap.task->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(va, level);
    assert(pte->is_enabled());
    page_table = pte->get_next_page().as<page_table_t>();
  }

  pte = page_table->walk(va, MEGA_PAGE_TABLE_LEVEL);
  assert(pte->is_enabled());

  page_table_cap.mapped         = true;
  page_table_cap.level          = KILO_PAGE;
  page_table_cap.virt_addr_base = va.raw();
  page_table_cap.parent_table   = pte->get_next_page().as<page_table_t>();

  return true;
}

int compare_id_cap(map_ptr<cap_slot_t> slot1, map_ptr<cap_slot_t> slot2) {
  assert(slot1 != nullptr);
  assert(get_cap_type(slot1->cap) == CAP_ID);
  assert(slot2 != nullptr);
  assert(get_cap_type(slot2->cap) == CAP_ID);

  auto& id_cap1 = slot1->cap.id;
  auto& id_cap2 = slot2->cap.id;

  if (id_cap1.val1 < id_cap2.val1) [[unlikely]] {
    return -1;
  }

  if (id_cap1.val1 > id_cap2.val1) [[unlikely]] {
    return 1;
  }

  if (id_cap1.val2 < id_cap2.val2) {
    return -1;
  }

  if (id_cap1.val2 > id_cap2.val2) {
    return 1;
  }

  if (id_cap1.val3 < id_cap2.val3) {
    return -1;
  }

  if (id_cap1.val3 > id_cap2.val3) {
    return 1;
  }

  return 0;
}
