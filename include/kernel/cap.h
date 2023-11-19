#ifndef KERNEL_CAP_H_
#define KERNEL_CAP_H_

#include <bit>
#include <cassert>

#include <kernel/address.h>
#include <kernel/page.h>
#include <libcaprese/cap.h>

struct task_t;
struct page_table_t;
struct cap_space_t;
struct cap_slot_t;

union capability_t {
  struct {
    uint64_t type   : 5;
    uint64_t unused1: 59;
    uint64_t unused2;
  } null;

  struct {
    uint64_t type      : 5;
    uint64_t device    : 1;
    uint64_t readable  : 1;
    uint64_t writable  : 1;
    uint64_t executable: 1;
    uint64_t size_bit  : 6;
    uint64_t phys_addr: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
    uint64_t used_size: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
  } memory;

  struct {
    uint64_t type             : 5;
    uint64_t killable         : 1;
    uint64_t switchable       : 1;
    uint64_t suspendable      : 1;
    uint64_t resumable        : 1;
    uint64_t register_gettable: 1;
    uint64_t register_settable: 1;
    uint64_t kill_notifiable  : 1;
    task_t*  task;
  } task;

  struct {
  } endpoint;

  struct {
    uint64_t      type : 5;
    uint64_t      level: 2;
    page_table_t* table;
  } page_table;

  struct {
    uint64_t    type      : 5;
    uint64_t    readable  : 1;
    uint64_t    writable  : 1;
    uint64_t    executable: 1;
    uint64_t    level     : 2;
    uint64_t    phys_addr: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
    virt_addr_t address;
  } virt_page;

  struct {
    uint64_t     type: 5;
    cap_space_t* space;
  } cap_space;

  struct {
    uint64_t    type    : 5;
    uint64_t    size_bit: 6;
    phys_addr_t address;
  } zombie;
};

static_assert(sizeof(capability_t) == sizeof(uint64_t) * 2);

constexpr cap_type_t get_cap_type(capability_t cap) {
  return static_cast<cap_type_t>(cap.null.type);
}

constexpr capability_t make_null_cap() {
  return {
    .null = {
      .type = static_cast<uint64_t>(CAP_NULL),
      .unused1 = 0,
      .unused2 = 0,
    },
  };
}

constexpr int MEMORY_CAP_DEVICE     = 1 << 0;
constexpr int MEMORY_CAP_READABLE   = 1 << 1;
constexpr int MEMORY_CAP_WRITABLE   = 1 << 2;
constexpr int MEMORY_CAP_EXECUTABLE = 1 << 3;

inline capability_t make_memory_cap(int flags, unsigned size_bit, phys_addr_t base_addr) {
  assert(base_addr.as<uintptr_t>() < CONFIG_MAX_PHYSICAL_ADDRESS);
  assert(size_bit < (1 << 6));

  return {
    .memory = {
      .type       = static_cast<uint64_t>(CAP_MEM),
      .device     = static_cast<uint64_t>((flags & MEMORY_CAP_DEVICE) >> 0),
      .readable   = static_cast<uint64_t>((flags & MEMORY_CAP_READABLE) >> 1),
      .writable   = static_cast<uint64_t>((flags & MEMORY_CAP_WRITABLE) >> 2),
      .executable = static_cast<uint64_t>((flags & MEMORY_CAP_EXECUTABLE) >> 3),
      .size_bit   = size_bit,
      .phys_addr  = base_addr.as<uintptr_t>(),
      .used_size = 0,
    },
  };
}

constexpr int TASK_CAP_KILLABLE          = 1 << 0;
constexpr int TASK_CAP_SWITCHABLE        = 1 << 1;
constexpr int TASK_CAP_SUSPENDABLE       = 1 << 2;
constexpr int TASK_CAP_RESUMABLE         = 1 << 3;
constexpr int TASK_CAP_REGISTER_GETTABLE = 1 << 4;
constexpr int TASK_CAP_REGISTER_SETTABLE = 1 << 5;
constexpr int TASK_CAP_KILL_NOTIFIABLE   = 1 << 6;

inline capability_t make_task_cap(int flags, task_t* task) {
  assert(task != nullptr);

  return {
    .task = {
      .type              = static_cast<uint64_t>(CAP_TASK),
      .killable          = static_cast<uint64_t>((flags & TASK_CAP_KILLABLE) >> 0),
      .switchable        = static_cast<uint64_t>((flags & TASK_CAP_SWITCHABLE) >> 1),
      .suspendable       = static_cast<uint64_t>((flags & TASK_CAP_SUSPENDABLE) >> 2),
      .resumable         = static_cast<uint64_t>((flags & TASK_CAP_RESUMABLE) >> 3),
      .register_gettable = static_cast<uint64_t>((flags & TASK_CAP_REGISTER_GETTABLE) >> 4),
      .register_settable = static_cast<uint64_t>((flags & TASK_CAP_REGISTER_SETTABLE) >> 5),
      .kill_notifiable   = static_cast<uint64_t>((flags & TASK_CAP_KILL_NOTIFIABLE) >> 6),
      .task              = task,
    },
  };
}

inline capability_t make_page_table_cap(uint64_t level, page_table_t* page_table) {
  assert(page_table != nullptr);
  assert(level <= MAX_PAGE_TABLE_LEVEL);

  return {
    .page_table = {
      .type  = static_cast<uint64_t>(CAP_PAGE_TABLE),
      .level = level,
      .table = page_table,
    },
  };
}

constexpr int VIRT_PAGE_CAP_READABLE   = 1 << 0;
constexpr int VIRT_PAGE_CAP_WRITABLE   = 1 << 1;
constexpr int VIRT_PAGE_CAP_EXECUTABLE = 1 << 2;

inline capability_t make_virt_page_cap(int flags, uint64_t level, uint64_t phys_addr) {
  assert(phys_addr < CONFIG_MAX_PHYSICAL_ADDRESS);
  assert(level <= MAX_PAGE_TABLE_LEVEL);

  return {
    .virt_page = {
      .type       = static_cast<uint64_t>(CAP_VIRT_PAGE),
      .readable   = static_cast<uint64_t>((flags & VIRT_PAGE_CAP_READABLE) >> 0),
      .writable   = static_cast<uint64_t>((flags & VIRT_PAGE_CAP_WRITABLE) >> 1),
      .executable = static_cast<uint64_t>((flags & VIRT_PAGE_CAP_EXECUTABLE) >> 2),
      .level      = level,
      .phys_addr  = phys_addr,
      .address    = virt_addr_t::from(nullptr),
    },
  };
}

inline capability_t make_cap_space_cap(cap_space_t* cap_space) {
  assert(cap_space != nullptr);

  return {
    .cap_space = {
      .type  = static_cast<uint64_t>(CAP_CAP_SPACE),
      .space = cap_space,
    },
  };
}

cap_slot_t* create_memory_object(cap_slot_t* dst, cap_slot_t* src, bool readable, bool writable, bool executable, size_t size, size_t alignment);
cap_slot_t* create_task_object(
    cap_slot_t* dst, cap_slot_t* src, cap_slot_t* cap_space_slot, cap_slot_t* root_page_table_slot, cap_slot_t* (&cap_space_page_table_slots)[NUM_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL]);
cap_slot_t* create_page_table_object(cap_slot_t* dst, cap_slot_t* src, uint64_t level);
cap_slot_t* create_virt_page_object(cap_slot_t* dst, cap_slot_t* src, bool readable, bool writable, bool executable, uint64_t level);
cap_slot_t* create_cap_space_object(cap_slot_t* dst, cap_slot_t* src);
cap_slot_t* create_object(task_t* task, cap_slot_t* cap_slot, cap_type_t type, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);

#endif // KERNEL_CAP_H_
