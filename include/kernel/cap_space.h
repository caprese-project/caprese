#ifndef KERNEL_CAP_SPACE_H_
#define KERNEL_CAP_SPACE_H_

#include <kernel/cap.h>
#include <kernel/page.h>

struct cap_slot_t {
  capability_t cap;
  cap_slot_t*  prev;
  cap_slot_t*  next;
};

struct alignas(PAGE_SIZE) cap_space_t {
  cap_slot_t slots[PAGE_SIZE / sizeof(cap_slot_t)];
};

static_assert(sizeof(cap_space_t) == PAGE_SIZE);

#endif // KERNEL_CAP_SPACE_H_
