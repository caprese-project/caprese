#ifndef KERNEL_CAP_SPACE_H_
#define KERNEL_CAP_SPACE_H_

#include <kernel/cap.h>
#include <kernel/page.h>

struct task_t;

struct cap_slot_t {
  capability_t        cap;
  map_ptr<cap_slot_t> prev;
  map_ptr<cap_slot_t> next;
};

struct alignas(PAGE_SIZE) cap_space_t {
  struct {
    map_ptr<cap_space_t> map;
    uintptr_t            space_index;
  } meta_info;

  cap_slot_t slots[PAGE_SIZE / sizeof(cap_slot_t) - 1];
};

static_assert(sizeof(cap_space_t) == PAGE_SIZE);

[[nodiscard]] bool insert_cap_space(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space);
[[nodiscard]] bool extend_cap_space(map_ptr<task_t> task, map_ptr<void> page);

[[nodiscard]] map_ptr<cap_slot_t> lookup_cap(map_ptr<task_t> task, uintptr_t cap_desc);
[[nodiscard]] size_t              get_cap_slot_index(map_ptr<cap_slot_t> cap_slot);

#endif // KERNEL_CAP_SPACE_H_
