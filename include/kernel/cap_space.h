#ifndef KERNEL_CAP_SPACE_H_
#define KERNEL_CAP_SPACE_H_

#include <kernel/cap.h>
#include <kernel/page.h>

struct task_t;
struct cap_space_t;

struct cap_slot_t {
  capability_t        cap;
  map_ptr<cap_slot_t> prev;
  map_ptr<cap_slot_t> next;

  [[nodiscard]] map_ptr<cap_space_t> get_cap_space() const;
  [[nodiscard]] bool                 is_unused() const;
  [[nodiscard]] bool                 is_isolated() const;
  [[nodiscard]] bool                 is_head() const;
  [[nodiscard]] bool                 is_tail() const;

  void                insert_before(map_ptr<cap_slot_t> slot);
  void                insert_after(map_ptr<cap_slot_t> slot);
  map_ptr<cap_slot_t> erase_this();
  void                replace(map_ptr<cap_slot_t> slot);
};

struct alignas(PAGE_SIZE) cap_space_t {
  struct {
    map_ptr<cap_space_t> map;
    map_ptr<task_t>      task;
    uintptr_t            space_index;
  } meta_info;

  cap_slot_t slots[PAGE_SIZE / sizeof(cap_slot_t) - 1];
};

static_assert(sizeof(cap_space_t) == PAGE_SIZE);

[[nodiscard]] bool           insert_cap_space(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space);
[[nodiscard]] virt_ptr<void> extend_cap_space(map_ptr<task_t> task, map_ptr<page_table_t> page);

[[nodiscard]] map_ptr<cap_slot_t> transfer_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot);
[[nodiscard]] map_ptr<cap_slot_t> delegate_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot);
[[nodiscard]] map_ptr<cap_slot_t> copy_cap(map_ptr<cap_slot_t> src_slot);
[[nodiscard]] bool                revoke_cap(map_ptr<cap_slot_t> slot);
[[nodiscard]] bool                destroy_cap(map_ptr<cap_slot_t> slot);
[[nodiscard]] bool                is_same_cap(map_ptr<cap_slot_t> lhs, map_ptr<cap_slot_t> rhs);

[[nodiscard]] map_ptr<cap_slot_t> lookup_cap(map_ptr<task_t> task, uintptr_t cap_desc);
[[nodiscard]] size_t              get_cap_slot_index(map_ptr<cap_slot_t> cap_slot);

#endif // KERNEL_CAP_SPACE_H_
