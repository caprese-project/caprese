#include <kernel/core_id.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_syscall(task_t* task) {
  syscall_args_t args;
  get_syscall_args(task, &args);

  uintptr_t ns = (args.code & (0xffff0000));
  uint16_t  id = static_cast<uint16_t>(args.code & (0x0000ffff));

  switch (ns) {
    case SYSNS_SYSTEM:
      return invoke_syscall_system(task, id, &args);
      break;
    case SYSNS_ARCH:
      return invoke_syscall_arch(task, id, &args);
      break;
    case SYSNS_CAP:
      return invoke_syscall_cap(task, id, &args);
      break;
    case SYSNS_MEM_CAP:
      return invoke_syscall_mem_cap(task, id, &args);
      break;
    case SYSNS_TASK_CAP:
      return invoke_syscall_task_cap(task, id, &args);
      break;
    default:
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_system([[maybe_unused]] task_t* task, uint16_t id, [[maybe_unused]] syscall_args_t* args) {
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
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_cap(task_t* task, uint16_t id, syscall_args_t* args) {
  switch (id) {
    case SYS_CAP_TYPE & 0xffff: {
      cap_slot_t* cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        return sysret_e_invalid_argument();
      }

      return sysret_s_ok(static_cast<uintptr_t>(get_cap_type(cap_slot->cap)));
    }
    default:
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_mem_cap(task_t* task, uint16_t id, syscall_args_t* args) {
  cap_slot_t* cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
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
      cap_slot_t* result = create_object(task, cap_slot, static_cast<cap_type_t>(args->args[1]), args->args[2], args->args[3], args->args[4], args->args[5], args->args[6]);
      if (result == nullptr) [[unlikely]] {
        return sysret_e_invalid_argument();
      }
      return sysret_s_ok(get_cap_slot_index(result));
    }
    default:
      return sysret_e_invalid_code();
  }
}

sysret_t invoke_syscall_task_cap(task_t* task, uint16_t id, syscall_args_t* args) {
  cap_slot_t* cap_slot = lookup_cap(task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    return sysret_e_invalid_argument();
  }
  if (get_cap_type(cap_slot->cap) != CAP_TASK) [[unlikely]] {
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
        return sysret_e_invalid_argument();
      }
      kill_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SWITCH & 0xffff:
      if (!task_cap.switchable) [[unlikely]] {
        return sysret_e_invalid_argument();
      }
      switch_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SUSPEND & 0xffff:
      if (!task_cap.suspendable) [[unlikely]] {
        return sysret_e_invalid_argument();
      }
      suspend_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_RESUME & 0xffff:
      if (!task_cap.resumable) [[unlikely]] {
        return sysret_e_invalid_argument();
      }
      resume_task(task_cap.task);
      return sysret_s_ok(0);
    default:
      return sysret_e_invalid_code();
  }
}
