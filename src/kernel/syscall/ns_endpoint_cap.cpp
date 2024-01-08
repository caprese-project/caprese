#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/ipc.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_endpoint_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/endpoint_cap";

  map_ptr<cap_slot_t> lookup_endpoint_cap(map_ptr<syscall_args_t> args) {
    map_ptr<task_t>& task = get_cls()->current_task;

    map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
    if (cap_slot == nullptr) [[unlikely]] {
      loge(tag, "Failed to look up cap: %d", args->args[0]);
      return 0_map;
    }

    if (get_cap_type(cap_slot->cap) != CAP_ENDPOINT) [[unlikely]] {
      loge(tag, "Cap is not an endpoint cap: %d", args->args[0]);
      errno = SYS_E_CAP_TYPE;
      return 0_map;
    }

    return cap_slot;
  }
} // namespace

sysret_t invoke_sys_endpoint_cap_send_short(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_send_short(true, cap_slot->cap.endpoint.endpoint, args->args[1], args->args[2], args->args[3], args->args[4], args->args[5], args->args[6])) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_send_long(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_send_long(true, cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_receive(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_receive(true, cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_reply(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_reply(cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_nb_send_short(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_send_short(false, cap_slot->cap.endpoint.endpoint, args->args[1], args->args[2], args->args[3], args->args[4], args->args[5], args->args[6])) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_nb_send_long(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_send_long(false, cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_nb_receive(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_receive(false, cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_call(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_call(cap_slot->cap.endpoint.endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_endpoint_cap_reply_and_receive(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_endpoint_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  // This endpoint cap itself could also be transferred.
  map_ptr<endpoint_t> endpoint = cap_slot->cap.endpoint.endpoint;

  if (!ipc_reply(endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!ipc_receive(true, endpoint, make_virt_ptr(args->args[1]))) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}
