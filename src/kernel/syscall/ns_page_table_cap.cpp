#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/log.h>
#include <kernel/syscall/ns_page_table_cap.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/page_table_cap";

  map_ptr<cap_slot_t> lookup_page_table_cap(map_ptr<syscall_args_t> args) {
    map_ptr<task_t>& task = get_cls()->current_task;

    map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
    if (cap_slot == nullptr) [[unlikely]] {
      loge(tag, "Failed to look up cap: %d", args->args[0]);
      return 0_map;
    }

    if (get_cap_type(cap_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
      loge(tag, "Cap is not a page table cap: %d", args->args[0]);
      errno = SYS_E_CAP_TYPE;
      return 0_map;
    }

    return cap_slot;
  }
} // namespace

sysret_t invoke_sys_page_table_cap_mapped(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  return sysret_s_ok(cap_slot->cap.page_table.mapped);
}

sysret_t invoke_sys_page_table_cap_level(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!cap_slot->cap.page_table.mapped) [[unlikely]] {
    loge(tag, "This page table cap is not mapped: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  return sysret_s_ok(cap_slot->cap.page_table.level);
}

sysret_t invoke_sys_page_table_cap_map_table(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> child_page_table_slot = lookup_cap(get_cls()->current_task, args->args[2]);

  if (!map_page_table_cap(cap_slot, args->args[1], child_page_table_slot)) [[unlikely]] {
    loge(tag, "Failed to map page table cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_page_table_cap_unmap_table(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> child_page_table_slot = lookup_cap(get_cls()->current_task, args->args[2]);

  if (!unmap_page_table_cap(cap_slot, args->args[1], child_page_table_slot)) [[unlikely]] {
    loge(tag, "Failed to unmap page table cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_page_table_cap_map_page(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> virt_page_slot = lookup_cap(get_cls()->current_task, args->args[5]);

  if (!map_virt_page_cap(cap_slot, args->args[1], virt_page_slot, args->args[2], args->args[3], args->args[4])) [[unlikely]] {
    loge(tag, "Failed to map virt page cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_page_table_cap_unmap_page(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> virt_page_slot = lookup_cap(get_cls()->current_task, args->args[2]);

  if (!unmap_virt_page_cap(cap_slot, args->args[1], virt_page_slot)) [[unlikely]] {
    loge(tag, "Failed to unmap virt page cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_page_table_cap_remap_page(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  map_ptr<cap_slot_t> virt_page_slot      = lookup_cap(get_cls()->current_task, args->args[5]);
  map_ptr<cap_slot_t> old_page_table_slot = lookup_cap(get_cls()->current_task, args->args[6]);

  if (!remap_virt_page_cap(cap_slot, args->args[1], virt_page_slot, args->args[2], args->args[3], args->args[4], old_page_table_slot)) [[unlikely]] {
    loge(tag, "Failed to remap virt page cap: %d", args->args[0]);
    return errno_to_sysret();
  }

  return sysret_s_ok(0);
}

sysret_t invoke_sys_page_table_cap_virt_addr_base(map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_page_table_cap(args);

  if (cap_slot == nullptr) [[unlikely]] {
    return errno_to_sysret();
  }

  if (!cap_slot->cap.page_table.mapped) [[unlikely]] {
    loge(tag, "This page table cap is not mapped: %d", args->args[0]);
    return sysret_e_cap_state();
  }

  return sysret_s_ok(cap_slot->cap.page_table.virt_addr_base);
}
