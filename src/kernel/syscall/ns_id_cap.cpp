#include <mutex>

#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_id_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/id_cap";
} // namespace

sysret_t invoke_sys_id_cap_create(map_ptr<syscall_args_t>) {
  map_ptr<task_t> task = get_cls()->current_task;

  std::lock_guard lock(task->lock);

  map_ptr<cap_slot_t> cap_slot = pop_free_slots(task);

  if (cap_slot == nullptr) [[unlikely]] {
    return sysret_e_out_of_cap_space();
  }

  map_ptr<cap_slot_t> result = create_id_object(cap_slot);

  if (result == nullptr) [[unlikely]] {
    push_free_slots(task, cap_slot);
    return errno_to_sysret();
  }

  return sysret_s_ok(get_cap_slot_index(result));
}

sysret_t invoke_sys_id_cap_compare(map_ptr<syscall_args_t> args) {
  map_ptr<task_t> task = get_cls()->current_task;

  std::lock_guard lock(task->lock);

  map_ptr<cap_slot_t> lhs_slot = lookup_cap(task, args->args[0]);

  if (lhs_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  if (get_cap_type(lhs_slot->cap) != CAP_ID) [[unlikely]] {
    loge(tag, "Cap is not an id cap: %d", args->args[0]);
    return sysret_e_cap_type();
  }

  map_ptr<cap_slot_t> rhs_slot = lookup_cap(task, args->args[1]);

  if (rhs_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  if (get_cap_type(rhs_slot->cap) != CAP_ID) [[unlikely]] {
    loge(tag, "Cap is not an id cap: %d", args->args[1]);
    return sysret_e_cap_type();
  }

  int result = compare_id_cap(lhs_slot, rhs_slot);

  return sysret_s_ok(static_cast<uintptr_t>(static_cast<intptr_t>(result)));
}
