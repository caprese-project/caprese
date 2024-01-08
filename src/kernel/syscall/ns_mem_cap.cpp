#include <kernel/cap.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_mem_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/mem_cap";

  map_ptr<cap_slot_t> lookup_mem_cap(map_ptr<syscall_args_t> args) {
    map_ptr<task_t>& task = get_cls()->current_task;

    map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
    if (cap_slot == nullptr) [[unlikely]] {
      loge(tag, "Failed to look up cap: %d", args->args[0]);
      return 0_map;
    }

    if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
      loge(tag, "Cap is not a memory cap: %d", args->args[0]);
      errno = SYS_E_CAP_TYPE;
      return 0_map;
    }

    return cap_slot;
  }
} // namespace

sysret_t invoke_sys_mem_cap_device(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_mem_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.memory.device);
}

sysret_t invoke_sys_mem_cap_phys_addr(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_mem_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.memory.phys_addr);
}

sysret_t invoke_sys_mem_cap_size(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_mem_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.memory.size);
}

sysret_t invoke_sys_mem_cap_used_size(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_mem_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.memory.used_size);
}

sysret_t invoke_sys_mem_cap_create_object(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_mem_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> result = create_object(get_cls()->current_task, cap_slot, static_cast<cap_type_t>(args->args[1]), args->args[2], args->args[3], args->args[4], args->args[5], args->args[6]);

  if (result == nullptr) [[unlikely]] {
    loge(tag, "Failed to create object: type=%d", args->args[1]);
    return errno_to_sysret();
  }

  return sysret_s_ok(get_cap_slot_index(result));
}
