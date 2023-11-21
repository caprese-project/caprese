#include <kernel/cls.h>
#include <kernel/core_id.h>
#include <kernel/frame.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>
#include <log/log.h>
#include <string.h>

namespace {
  constexpr const char* tag = "kernel/syscall";
} // namespace

sysret_t invoke_syscall() {
  syscall_args_t args {};
  get_syscall_args(make_map_ptr(&args));

  uintptr_t ns = (args.code & (0xffff0000));
  uint16_t  id = static_cast<uint16_t>(args.code & (0x0000ffff));

  switch (ns) {
    case SYSNS_SYSTEM:
      return invoke_syscall_system(id, make_map_ptr(&args));
    case SYSNS_ARCH:
      return invoke_syscall_arch(id, make_map_ptr(&args));
    case SYSNS_CAP:
      return invoke_syscall_cap(id, make_map_ptr(&args));
    case SYSNS_MEM_CAP:
      return invoke_syscall_mem_cap(id, make_map_ptr(&args));
    case SYSNS_TASK_CAP:
      return invoke_syscall_task_cap(id, make_map_ptr(&args));
    case SYSNS_PAGE_TABLE_CAP:
      return invoke_syscall_page_table_cap(id, make_map_ptr(&args));
    case SYSNS_VIRT_PAGE_CAP:
      return invoke_syscall_virt_page_cap(id, make_map_ptr(&args));
    default:
      loge(tag, "Invalid syscall namespace: 0x%x", ns);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_system(uint16_t id, [[maybe_unused]] map_ptr<syscall_args_t> args) {
  switch (id) {
    case SYS_SYSTEM_NULL:
      return sysret_s_ok(0);
    case SYS_SYSTEM_CORE_ID:
      return sysret_s_ok(get_core_id());
    case SYS_SYSTEM_PAGE_SIZE:
      return sysret_s_ok(PAGE_SIZE);
    case SYS_SYSTEM_USER_SPACE_START:
      return sysret_s_ok(CONFIG_USER_SPACE_BASE);
    case SYS_SYSTEM_USER_SPACE_END:
      return sysret_s_ok(CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE);
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  switch (id) {
    case SYS_CAP_TYPE & 0xffff: {
      map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }

      return sysret_s_ok(static_cast<uintptr_t>(get_cap_type(cap_slot->cap)));
    }
    case SYS_CAP_COPY & 0xffff: {
      map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }

      map_ptr<cap_slot_t> result = copy_cap(cap_slot);
      if (result == nullptr) [[unlikely]] {
        loge(tag, "Failed to copy cap: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }

      return sysret_s_ok(get_cap_slot_index(result));
    }
    case SYS_CAP_REVOKE & 0xffff: {
      map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }

      if (revoke_cap(cap_slot)) {
        return sysret_s_ok(0);
      } else {
        loge(tag, "Failed to revoke cap: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_mem_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_invalid_argument();
  }

  auto& mem_cap = cap_slot->cap.memory;

  switch (id) {
    case SYS_MEM_CAP_DEVICE & 0xffff:
      return sysret_s_ok(mem_cap.device);
    case SYS_MEM_CAP_READABLE & 0xffff:
      return sysret_s_ok(mem_cap.readable);
    case SYS_MEM_CAP_WRITABLE & 0xffff:
      return sysret_s_ok(mem_cap.writable);
    case SYS_MEM_CAP_EXECUTABLE & 0xffff:
      return sysret_s_ok(mem_cap.executable);
    case SYS_MEM_CAP_PHYS_ADDR & 0xffff:
      return sysret_s_ok(mem_cap.phys_addr);
    case SYS_MEM_CAP_SIZE_BIT & 0xffff:
      return sysret_s_ok(mem_cap.size_bit);
    case SYS_MEM_CAP_USED_SIZE & 0xffff:
      return sysret_s_ok(mem_cap.used_size);
    case SYS_MEM_CAP_CREATE_OBJECT & 0xffff: {
      map_ptr<cap_slot_t> result = create_object(get_cls()->current_task, cap_slot, static_cast<cap_type_t>(args->args[1]), args->args[2], args->args[3], args->args[4], args->args[5], args->args[6]);
      if (result == nullptr) [[unlikely]] {
        loge(tag, "Failed to create object: type=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(get_cap_slot_index(result));
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_task_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_TASK) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_invalid_argument();
  }

  auto& task_cap = cap_slot->cap.task;

  switch (id) {
    case SYS_TASK_CAP_KILLABLE & 0xffff:
      return sysret_s_ok(task_cap.killable);
    case SYS_TASK_CAP_SWITCHABLE & 0xffff:
      return sysret_s_ok(task_cap.switchable);
    case SYS_TASK_CAP_SUSPENDABLE & 0xffff:
      return sysret_s_ok(task_cap.suspendable);
    case SYS_TASK_CAP_RESUMABLE & 0xffff:
      return sysret_s_ok(task_cap.resumable);
    case SYS_TASK_CAP_KILL & 0xffff:
      if (!task_cap.killable) [[unlikely]] {
        loge(tag, "This task cap is not killable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      kill_task(task_cap.task, static_cast<int>(args->args[1]));
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SWITCH & 0xffff:
      if (!task_cap.switchable) [[unlikely]] {
        loge(tag, "This task cap is not switchable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      switch_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SUSPEND & 0xffff:
      if (!task_cap.suspendable) [[unlikely]] {
        loge(tag, "This task cap is not suspendable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      suspend_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_RESUME & 0xffff:
      if (!task_cap.resumable) [[unlikely]] {
        loge(tag, "This task cap is not resumable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      resume_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_GET_REG & 0xffff:
      if (!task_cap.register_gettable) [[unlikely]] {
        loge(tag, "This task cap is not register gettable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
        return sysret_e_invalid_argument();
      }
      if (args->args[1] > LAST_REGISTER) [[unlikely]] {
        loge(tag, "Invalid register index: %d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(get_register(make_map_ptr(&task_cap.task->frame), args->args[1]));
    case SYS_TASK_CAP_SET_REG & 0xffff:
      if (!task_cap.register_settable) [[unlikely]] {
        loge(tag, "This task cap is not register settable: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
        return sysret_e_invalid_argument();
      }
      if (args->args[1] > LAST_REGISTER) [[unlikely]] {
        loge(tag, "Invalid register index: %d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(set_register(make_map_ptr(&task_cap.task->frame), args->args[1], args->args[2]));
    case SYS_TASK_CAP_TRANSFER_CAP & 0xffff: {
      map_ptr<cap_slot_t> dst_task_slot = lookup_cap(task_cap.task, args->args[1]);
      if (dst_task_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      if (get_cap_type(dst_task_slot->cap) != CAP_TASK) [[unlikely]] {
        loge(tag, "Invalid cap type: %d", get_cap_type(dst_task_slot->cap));
        return sysret_e_invalid_argument();
      }

      map_ptr<cap_slot_t> src_slot = lookup_cap(task_cap.task, args->args[2]);
      if (src_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[2]);
        return sysret_e_invalid_argument();
      }

      map_ptr<cap_slot_t> dst_slot = transfer_cap(dst_task_slot->cap.task.task, src_slot);
      if (dst_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to transfer cap: %d", args->args[2]);
        return sysret_e_invalid_argument();
      }

      return sysret_s_ok(get_cap_slot_index(dst_slot));
    }
    case SYS_TASK_CAP_DELEGATE_CAP & 0xffff: {
      map_ptr<cap_slot_t> dst_task_slot = lookup_cap(task_cap.task, args->args[1]);
      if (dst_task_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      if (get_cap_type(dst_task_slot->cap) != CAP_TASK) [[unlikely]] {
        loge(tag, "Invalid cap type: %d", get_cap_type(dst_task_slot->cap));
        return sysret_e_invalid_argument();
      }

      map_ptr<cap_slot_t> src_slot = lookup_cap(task_cap.task, args->args[2]);
      if (src_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[2]);
        return sysret_e_invalid_argument();
      }

      map_ptr<cap_slot_t> dst_slot = delegate_cap(dst_task_slot->cap.task.task, src_slot);
      if (dst_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to delegate cap: %d", args->args[2]);
        return sysret_e_invalid_argument();
      }

      return sysret_s_ok(get_cap_slot_index(dst_slot));
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_page_table_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_invalid_argument();
  }

  auto& page_table_cap = cap_slot->cap.page_table;

  switch (id) {
    case SYS_PAGE_TABLE_CAP_MAPPED & 0xffff:
      return sysret_s_ok(page_table_cap.mapped);
    case SYS_PAGE_TABLE_CAP_LEVEL & 0xffff:
      if (!page_table_cap.mapped) [[unlikely]] {
        loge(tag, "This page table cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(page_table_cap.level);
    case SYS_PAGE_TABLE_CAP_MAP_TABLE & 0xffff:
      if (!map_page_table_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to map page table: index=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_UNMAP_TABLE & 0xffff:
      if (!unmap_page_table_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to unmap page table: index=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_MAP_PAGE & 0xffff:
      if (!map_virt_page_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[5]), args->args[2], args->args[3], args->args[4])) [[unlikely]] {
        loge(tag, "Failed to map virt page: index=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_UNMAP_PAGE & 0xffff:
      if (!unmap_virt_page_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to unmap virt page: index=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_REMAP_PAGE & 0xffff:
      if (!remap_virt_page_cap(cap_slot,
                               args->args[1],
                               lookup_cap(get_cls()->current_task, args->args[5]),
                               args->args[2],
                               args->args[3],
                               args->args[4],
                               lookup_cap(get_cls()->current_task, args->args[6]))) [[unlikely]] {
        loge(tag, "Failed to remap virt page: index=%d", args->args[1]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(0);
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_virt_page_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_invalid_argument();
  }

  auto& virt_page_cap = cap_slot->cap.virt_page;

  switch (id) {
    case SYS_VIRT_PAGE_CAP_MAPPED & 0xffff:
      return sysret_s_ok(virt_page_cap.mapped);
    case SYS_VIRT_PAGE_CAP_READABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.readable);
    case SYS_VIRT_PAGE_CAP_WRITABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.writable);
    case SYS_VIRT_PAGE_CAP_EXECUTABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.executable);
    case SYS_VIRT_PAGE_CAP_LEVEL & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.level);
    case SYS_VIRT_PAGE_CAP_PHYS_ADDR & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.phys_addr);
    case SYS_VIRT_PAGE_CAP_VIRT_ADDR & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(virt_page_cap.address.raw());
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_invalid_code();
  }
}
