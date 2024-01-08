#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_virt_page_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/virt_page_cap";

  map_ptr<cap_slot_t> lookup_virt_page_cap(map_ptr<syscall_args_t> args) {
    map_ptr<task_t>& task = get_cls()->current_task;

    map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
    if (cap_slot == nullptr) [[unlikely]] {
      loge(tag, "Failed to look up cap: %d", args->args[0]);
      return 0_map;
    }

    if (get_cap_type(cap_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
      loge(tag, "Cap is not a virt page cap: %d", args->args[0]);
      errno = SYS_E_CAP_TYPE;
      return 0_map;
    }

    return cap_slot;
  }
} // namespace

sysret_t invoke_sys_virt_page_cap_mapped(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.mapped);
}

sysret_t invoke_sys_virt_page_cap_readable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.readable);
}

sysret_t invoke_sys_virt_page_cap_writable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.writable);
}

sysret_t invoke_sys_virt_page_cap_executable(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.executable);
}

sysret_t invoke_sys_virt_page_cap_level(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.level);
}

sysret_t invoke_sys_virt_page_cap_phys_addr(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.phys_addr);
}

sysret_t invoke_sys_virt_page_cap_virt_addr(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_virt_page_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!cap_slot->cap.virt_page.mapped) [[unlikely]] {
    loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  return sysret_s_ok(cap_slot->cap.virt_page.address.raw());
}
