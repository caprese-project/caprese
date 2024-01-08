#include <bit>
#include <mutex>

#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_task_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/task_cap";

  map_ptr<cap_slot_t> lookup_task_cap(map_ptr<syscall_args_t> args) {
    map_ptr<task_t>& task = get_cls()->current_task;

    map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
    if (cap_slot == nullptr) [[unlikely]] {
      loge(tag, "Failed to look up cap: %d", args->args[0]);
      return 0_map;
    }

    if (get_cap_type(cap_slot->cap) != CAP_TASK) [[unlikely]] {
      loge(tag, "Cap is not a task cap: %d", args->args[0]);
      errno = SYS_E_CAP_TYPE;
      return 0_map;
    }

    return cap_slot;
  }
} // namespace

sysret_t invoke_sys_task_cap_tid(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(std::bit_cast<uint32_t>(cap_slot->cap.task.task->tid));
}

sysret_t invoke_sys_task_cap_killable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.killable);
}

sysret_t invoke_sys_task_cap_switchable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.switchable);
}

sysret_t invoke_sys_task_cap_suspendable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.suspendable);
}

sysret_t invoke_sys_task_cap_resumable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.resumable);
}

sysret_t invoke_sys_task_cap_kill(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.killable) [[unlikely]] {
    loge(tag, "This task cap is not killable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  kill_task(task_cap.task, static_cast<int>(args->args[1]));

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_switch(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.switchable) [[unlikely]] {
    loge(tag, "This task cap is not switchable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  {
    std::lock_guard lock(task_cap.task->lock);

    if (task_cap.task->state != task_state_t::ready) [[unlikely]] {
      loge(tag, "This task is not ready: %d", args->args[0]);
      return sysret_e_ill_state();
    }

    remove_ready_queue(task_cap.task);
  }

  switch_task(task_cap.task);

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_suspend(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.suspendable) [[unlikely]] {
    loge(tag, "This task cap is not suspendable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  suspend_task(task_cap.task);

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_resume(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.resumable) [[unlikely]] {
    loge(tag, "This task cap is not resumable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  resume_task(task_cap.task);

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_get_reg(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.register_gettable) [[unlikely]] {
    loge(tag, "This task cap is not register gettable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
    loge(tag, "This task is not suspended: %d", args->args[0]);
    return sysret_e_ill_state();
  }

  if (args->args[1] > LAST_REGISTER) [[unlikely]] {
    loge(tag, "Invalid register index: %d", args->args[1]);
    return sysret_e_ill_args();
  }

  uintptr_t value = get_register(make_map_ptr(&task_cap.task->frame), args->args[1]);

  return sysret_s_ok(value);
}

sysret_t invoke_sys_task_cap_set_reg(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (!task_cap.register_settable) [[unlikely]] {
    loge(tag, "This task cap is not register settable: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
    loge(tag, "This task is not suspended: %d", args->args[0]);
    return sysret_e_ill_state();
  }

  if (args->args[1] > LAST_REGISTER) [[unlikely]] {
    loge(tag, "Invalid register index: %d", args->args[1]);
    return sysret_e_ill_args();
  }

  uintptr_t old_value = set_register(make_map_ptr(&task_cap.task->frame), args->args[1], args->args[2]);

  return sysret_s_ok(old_value);
}

sysret_t invoke_sys_task_cap_transfer_cap(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
    loge(tag, "This task is not suspended: %d", args->args[0]);
    return sysret_e_ill_state();
  }

  map_ptr<cap_slot_t> src_slot = lookup_cap(get_cls()->current_task, args->args[1]);
  if (src_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> dst_slot = transfer_cap(task_cap.task, src_slot);
  if (dst_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to transfer cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(get_cap_slot_index(dst_slot));
}

sysret_t invoke_sys_task_cap_delegate_cap(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  auto& task_cap = cap_slot->cap.task;

  if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
    loge(tag, "This task is not suspended: %d", args->args[0]);
    return sysret_e_ill_state();
  }

  map_ptr<cap_slot_t> src_slot = lookup_cap(get_cls()->current_task, args->args[1]);
  if (src_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> dst_slot = delegate_cap(task_cap.task, src_slot);
  if (dst_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to delegate cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(get_cap_slot_index(dst_slot));
}

sysret_t invoke_sys_task_cap_get_free_slot_count(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.task->free_slots_count);
}

sysret_t invoke_sys_task_cap_get_cap_space_count(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.task->cap_count.num_cap_space);
}

sysret_t invoke_sys_task_cap_get_cap_space_ext_count(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.task.task->cap_count.num_extension);
}

sysret_t invoke_sys_task_cap_insert_cap_space(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> cap_space_slot = lookup_cap(get_cls()->current_task, args->args[1]);

  if (cap_space_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  if (get_cap_type(cap_space_slot->cap) != CAP_CAP_SPACE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_space_slot->cap));
    return sysret_e_cap_type();
  }

  if (!insert_cap_space(cap_slot, cap_space_slot)) [[unlikely]] {
    loge(tag, "Failed to insert cap space: %d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_extend_cap_space(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> page_table_slot = lookup_cap(get_cls()->current_task, args->args[1]);

  if (page_table_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  if (get_cap_type(page_table_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(page_table_slot->cap));
    return sysret_e_cap_type();
  }

  if (!extend_cap_space(cap_slot, page_table_slot)) [[unlikely]] {
    loge(tag, "Failed to extend cap space: %d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_task_cap_set_kill_notify(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_task_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> ep_cap_slot = lookup_cap(get_cls()->current_task, args->args[1]);

  if (ep_cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[1]);
    return errno_to_sysret();
  }

  if (get_cap_type(ep_cap_slot->cap) != CAP_ENDPOINT) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(ep_cap_slot->cap));
    return sysret_e_cap_type();
  }

  set_kill_notify(cap_slot->cap.task.task, ep_cap_slot->cap.endpoint.endpoint);

  return sysret_s_ok(0);
}
