#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/cap";
} // namespace

sysret_t invoke_sys_cap_type(map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    if (errno == SYS_S_OK) {
      return sysret_s_ok(static_cast<uintptr_t>(CAP_NULL));
    }

    loge(tag, "Failed to look up cap: %d", args->args[0]);

    return errno_to_sysret();
  }

  return sysret_s_ok(static_cast<uintptr_t>(get_cap_type(cap_slot->cap)));
}

sysret_t invoke_sys_cap_copy(map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> result = copy_cap(cap_slot);
  if (result == nullptr) [[unlikely]] {
    loge(tag, "Failed to copy cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(get_cap_slot_index(result));
}

sysret_t invoke_sys_cap_revoke(map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  if (!revoke_cap(cap_slot)) [[unlikely]] {
    loge(tag, "Failed to revoke cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_cap_destroy(map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  if (!destroy_cap(cap_slot)) [[unlikely]] {
    loge(tag, "Failed to destroy cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_cap_same(map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  map_ptr<cap_slot_t> lhs_cap_slot = lookup_cap(task, args->args[0]);
  if (lhs_cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> rhs_cap_slot = lookup_cap(task, args->args[1]);
  if (rhs_cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(is_same_cap(lhs_cap_slot, rhs_cap_slot));
}
