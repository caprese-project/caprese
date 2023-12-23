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
struct endpoint_t;

union capability_t {
  struct {
    uint64_t type   : 5;
    uint64_t unused1: 59;
    uint64_t unused2;
    uint64_t unused3;
  } null;

  struct {
    uint64_t type  : 5;
    uint64_t device: 1;
    uint64_t size: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
    uint64_t phys_addr: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
    uint64_t used_size: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
  } memory;

  struct {
    uint64_t        type             : 5;
    uint64_t        killable         : 1;
    uint64_t        switchable       : 1;
    uint64_t        suspendable      : 1;
    uint64_t        resumable        : 1;
    uint64_t        register_gettable: 1;
    uint64_t        register_settable: 1;
    uint64_t        kill_notifiable  : 1;
    map_ptr<task_t> task;
    uint64_t        unused;
  } task;

  struct {
    uint64_t            type      : 5;
    uint64_t            sendable  : 1;
    uint64_t            receivable: 1;
    map_ptr<endpoint_t> endpoint;
    uint64_t            unused;
  } endpoint;

  struct {
    uint64_t              type  : 5;
    uint64_t              mapped: 1;
    uint64_t              level : 2;
    uint64_t              virt_addr_base: std::countr_zero<uint64_t>(CONFIG_MAX_VIRTUAL_ADDRESS);
    map_ptr<page_table_t> table;
    map_ptr<page_table_t> parent_table;
  } page_table;

  struct {
    uint64_t              type      : 5;
    uint64_t              mapped    : 1;
    uint64_t              readable  : 1;
    uint64_t              writable  : 1;
    uint64_t              executable: 1;
    uint64_t              level     : 2;
    uint64_t              index: std::countr_zero<uint64_t>(NUM_PAGE_TABLE_ENTRY);
    uint64_t              phys_addr: std::countr_zero<uint64_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
    virt_ptr<void>        address;
    map_ptr<page_table_t> parent_table;
  } virt_page;

  struct {
    uint64_t             type: 5;
    uint64_t             used: 1;
    map_ptr<cap_space_t> space;
    uint64_t             unused;
  } cap_space;

  struct {
    uint64_t type: 5;
    uint64_t val1: 59;
    uint64_t val2;
    uint64_t val3;
  } id;

  struct {
    uint64_t type   : 5;
    uint64_t unused1: 59;
    uint64_t unused2;
    uint64_t unused3;
  } zombie;
};

static_assert(sizeof(capability_t) == sizeof(uint64_t) * 3);

constexpr size_t get_cap_size(cap_type_t type) {
  switch (type) {
    case CAP_NULL:
      return 0;
    case CAP_MEM:
      return 0;
    case CAP_TASK:
      return PAGE_SIZE;
    case CAP_ENDPOINT:
      return 64;
    case CAP_PAGE_TABLE:
      return PAGE_SIZE;
    case CAP_VIRT_PAGE:
      return PAGE_SIZE;
    case CAP_CAP_SPACE:
      return PAGE_SIZE;
    case CAP_ID:
      return 0;
    case CAP_ZOMBIE:
      return 0;
    case CAP_UNKNOWN:
      return -1;
  }

  return -1;
}

constexpr size_t get_cap_align(cap_type_t type) {
  switch (type) {
    case CAP_NULL:
      return 0;
    case CAP_MEM:
      return 1;
    case CAP_TASK:
      return PAGE_SIZE;
    case CAP_ENDPOINT:
      return 8;
    case CAP_PAGE_TABLE:
      return PAGE_SIZE;
    case CAP_VIRT_PAGE:
      return PAGE_SIZE;
    case CAP_CAP_SPACE:
      return PAGE_SIZE;
    case CAP_ID:
      return 0;
    case CAP_ZOMBIE:
      return 0;
    case CAP_UNKNOWN:
      return -1;
  }

  return -1;
}

constexpr cap_type_t get_cap_type(capability_t cap) {
  return static_cast<cap_type_t>(cap.null.type);
}

constexpr capability_t make_null_cap() {
  return {
    .null = {
      .type    = static_cast<uint64_t>(CAP_NULL),
      .unused1 = 0,
      .unused2 = 0,
      .unused3 = 0,
    },
  };
}

inline capability_t make_memory_cap(bool device, size_t size, phys_ptr<void> base_addr) {
  assert(base_addr < CONFIG_MAX_PHYSICAL_ADDRESS);
  assert(size < CONFIG_MAX_PHYSICAL_ADDRESS);

  return {
    .memory = {
      .type       = static_cast<uint64_t>(CAP_MEM),
      .device     = static_cast<uint64_t>(device),
      .size       = size,
      .phys_addr  = base_addr.raw(),
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

inline capability_t make_task_cap(int flags, map_ptr<task_t> task) {
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
      .unused            = 0,
    },
  };
}

inline capability_t make_endpoint_cap(map_ptr<endpoint_t> endpoint) {
  assert(endpoint != nullptr);

  return {
    .endpoint = {
      .type       = static_cast<uint64_t>(CAP_ENDPOINT),
      .sendable   = 1,
      .receivable = 1,
      .endpoint   = endpoint,
      .unused     = 0,
    },
  };
}

inline capability_t make_page_table_cap(map_ptr<page_table_t> page_table, bool mapped, uint64_t level, virt_ptr<void> virt_addr_base, map_ptr<page_table_t> parent_table) {
  assert(page_table != nullptr);
  assert(!mapped || virt_addr_base.raw() % get_page_size(level + 1) == 0);
  assert(!mapped || level == MAX_PAGE_TABLE_LEVEL || parent_table != nullptr);

  return {
    .page_table = {
      .type           = static_cast<uint64_t>(CAP_PAGE_TABLE),
      .mapped         = mapped,
      .level          = level,
      .virt_addr_base = virt_addr_base.raw(),
      .table          = page_table,
      .parent_table   = parent_table,
    },
  };
}

inline capability_t make_virt_page_cap(
    bool readable, bool writable, bool executable, bool mapped, uint64_t level, phys_ptr<void> phys_addr, virt_ptr<void> virt_addr, map_ptr<page_table_t> parent_table) {
  assert(phys_addr < CONFIG_MAX_PHYSICAL_ADDRESS);
  assert(level <= MAX_PAGE_TABLE_LEVEL);

  return {
    .virt_page = {
      .type         = static_cast<uint64_t>(CAP_VIRT_PAGE),
      .mapped       = mapped,
      .readable     = readable,
      .writable     = writable,
      .executable   = executable,
      .level        = level,
      .index        = get_page_table_index(virt_addr, level),
      .phys_addr    = phys_addr.raw(),
      .address      = virt_addr.raw(),
      .parent_table = parent_table,
    },
  };
}

inline capability_t make_cap_space_cap(map_ptr<cap_space_t> cap_space, bool used) {
  assert(cap_space != nullptr);

  return {
    .cap_space = {
      .type   = static_cast<uint64_t>(CAP_CAP_SPACE),
      .used   = used,
      .space  = cap_space,
      .unused = 0,
    },
  };
}

inline capability_t make_id_cap(uint64_t val1, uint64_t val2, uint64_t val3) {
  assert(val1 < (1ull << 59));
  return {
    .id = {
      .type = static_cast<uint64_t>(CAP_ID),
      .val1 = val1,
      .val2 = val2,
      .val3 = val3,
    },
  };
}

inline capability_t make_zombie_cap() {
  return {
    .zombie = {
      .type    = static_cast<uint64_t>(CAP_ZOMBIE),
      .unused1 = 0,
      .unused2 = 0,
      .unused3 = 0,
    },
  };
}

capability_t make_unique_id_cap();

map_ptr<cap_slot_t> create_memory_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, size_t size, size_t alignment);
map_ptr<cap_slot_t> create_task_object(map_ptr<cap_slot_t> dst,
                                       map_ptr<cap_slot_t> src,
                                       map_ptr<cap_slot_t> cap_space_slot,
                                       map_ptr<cap_slot_t> root_page_table_slot,
                                       map_ptr<cap_slot_t> (&cap_space_page_table_slots)[NUM_INTER_PAGE_TABLE + 1]);
map_ptr<cap_slot_t> create_endpoint_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src);
map_ptr<cap_slot_t> create_page_table_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src);
map_ptr<cap_slot_t> create_virt_page_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src, bool readable, bool writable, bool executable, uint64_t level);
map_ptr<cap_slot_t> create_cap_space_object(map_ptr<cap_slot_t> dst, map_ptr<cap_slot_t> src);
map_ptr<cap_slot_t> create_id_object(map_ptr<cap_slot_t> dst);
map_ptr<cap_slot_t> create_object(map_ptr<task_t> task, map_ptr<cap_slot_t> cap_slot, cap_type_t type, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);

bool is_same_object(map_ptr<cap_slot_t> lhs, map_ptr<cap_slot_t> rhs);

void destroy_memory_object(map_ptr<cap_slot_t> slot);
void destroy_task_object(map_ptr<cap_slot_t> slot);
void destroy_endpoint_object(map_ptr<cap_slot_t> slot);
void destroy_page_table_object(map_ptr<cap_slot_t> slot);
void destroy_virt_page_object(map_ptr<cap_slot_t> slot);
void destroy_cap_space_object(map_ptr<cap_slot_t> slot);
void destroy_id_object(map_ptr<cap_slot_t> slot);

bool map_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot);
bool unmap_page_table_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> child_page_table_slot);
bool map_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable);
bool unmap_virt_page_cap(map_ptr<cap_slot_t> page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot);
bool remap_virt_page_cap(
    map_ptr<cap_slot_t> new_page_table_slot, size_t index, map_ptr<cap_slot_t> virt_page_slot, bool readable, bool writable, bool executable, map_ptr<cap_slot_t> old_page_table_slot);

bool insert_cap_space(map_ptr<cap_slot_t> task_slot, map_ptr<cap_slot_t> cap_space_slot);
bool extend_cap_space(map_ptr<cap_slot_t> task_slot, map_ptr<cap_slot_t> page_table_slot);

int compare_id_cap(map_ptr<cap_slot_t> slot1, map_ptr<cap_slot_t> slot2);

#endif // KERNEL_CAP_H_
