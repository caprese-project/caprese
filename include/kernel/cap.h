#ifndef KERNEL_CAP_H_
#define KERNEL_CAP_H_

#include <bit>
#include <cassert>

#include <kernel/address.h>

struct task_t;
struct page_table_t;
struct cap_space_t;

enum struct cap_type_t : uint64_t {
  null       = 0,
  memory     = 1,
  task       = 2,
  endpoint   = 3,
  page_table = 4,
  virt_page  = 5,
  cap_space  = 6,
  zombie     = 7,
};

union cap_t {
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
    uint64_t used_count: std::countr_zero<uintptr_t>(CONFIG_MAX_PHYSICAL_ADDRESS);
  } memory;

  struct {
    uint64_t type              : 5;
    uint64_t killable          : 1;
    uint64_t switchable        : 1;
    uint64_t suspendable       : 1;
    uint64_t resumable         : 1;
    uint64_t cap_transferable  : 1;
    uint64_t message_sendable  : 1;
    uint64_t register_gettable : 1;
    uint64_t register_settable : 1;
    uint64_t cap_space_gettable: 1;
    uint64_t cap_space_settable: 1;
    uint64_t kill_notifiable   : 1;
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

constexpr cap_t make_null_cap() {
  return {
    .null = {
      .type = static_cast<uint64_t>(cap_type_t::null),
      .unused1 = 0,
      .unused2 = 0,
    },
  };
}

constexpr int MEMORY_CAP_DEVICE     = 1 << 0;
constexpr int MEMORY_CAP_READABLE   = 1 << 1;
constexpr int MEMORY_CAP_WRITABLE   = 1 << 2;
constexpr int MEMORY_CAP_EXECUTABLE = 1 << 3;

inline cap_t make_memory_cap(int flags, unsigned size_bit, phys_addr_t base_addr) {
  assert(base_addr.as<uintptr_t>() < CONFIG_MAX_PHYSICAL_ADDRESS);
  assert(size_bit < (1 << 6));

  return {
    .memory = {
      .type       = static_cast<uint64_t>(cap_type_t::memory),
      .device     = (flags & MEMORY_CAP_DEVICE) != 0,
      .readable   = (flags & MEMORY_CAP_READABLE) != 0,
      .writable   = (flags & MEMORY_CAP_WRITABLE) != 0,
      .executable = (flags & MEMORY_CAP_EXECUTABLE) != 0,
      .size_bit   = size_bit,
      .phys_addr  = base_addr.as<uintptr_t>(),
      .used_count = 0,
    },
  };
}

#endif // KERNEL_CAP_H_
