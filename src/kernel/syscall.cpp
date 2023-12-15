#include <cstring>
#include <iterator>
#include <mutex>

#include <kernel/cls.h>
#include <kernel/core_id.h>
#include <kernel/frame.h>
#include <kernel/ipc.h>
#include <kernel/log.h>
#include <kernel/syscall.h>
#include <kernel/task.h>
#include <kernel/user_ptr.h>
#include <libcaprese/syscall.h>

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
    case SYSNS_ENDPOINT_CAP:
      return invoke_syscall_endpoint_cap(id, make_map_ptr(&args));
    case SYSNS_PAGE_TABLE_CAP:
      return invoke_syscall_page_table_cap(id, make_map_ptr(&args));
    case SYSNS_VIRT_PAGE_CAP:
      return invoke_syscall_virt_page_cap(id, make_map_ptr(&args));
    case SYSNS_ID_CAP:
      return invoke_syscall_id_cap(id, make_map_ptr(&args));
    default:
      loge(tag, "Invalid syscall namespace: 0x%x", ns);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_system(uint16_t id, map_ptr<syscall_args_t> args) {
  switch (id) {
    case SYS_SYSTEM_NULL & 0xffff:
      return sysret_s_ok(0);
    case SYS_SYSTEM_CORE_ID & 0xffff:
      return sysret_s_ok(get_core_id());
    case SYS_SYSTEM_PAGE_SIZE & 0xffff:
      return sysret_s_ok(PAGE_SIZE);
    case SYS_SYSTEM_USER_SPACE_START & 0xffff:
      return sysret_s_ok(CONFIG_USER_SPACE_BASE);
    case SYS_SYSTEM_USER_SPACE_END & 0xffff:
      return sysret_s_ok(CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE);
    case SYS_SYSTEM_CAPS_PER_CAP_SPACE & 0xffff:
      return sysret_s_ok(std::size(static_cast<cap_space_t*>(nullptr)->slots));
    case SYS_SYSTEM_YIELD & 0xffff:
      yield();
      return sysret_s_ok(0);
    case SYS_SYSTEM_CAP_SIZE & 0xffff:
      if (args->args[0] > CAP_ZOMBIE) {
        loge(tag, "Invalid cap type: %d", args->args[0]);
        return sysret_e_ill_args();
      }
      return sysret_s_ok(get_cap_size(static_cast<cap_type_t>(args->args[0])));
    case SYS_SYSTEM_CAP_ALIGN & 0xffff:
      if (args->args[0] > CAP_ZOMBIE) {
        loge(tag, "Invalid cap type: %d", args->args[0]);
        return sysret_e_ill_args();
      }
      return sysret_s_ok(get_cap_align(static_cast<cap_type_t>(args->args[0])));
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<task_t>& task = get_cls()->current_task;

  switch (id) {
    case SYS_CAP_TYPE & 0xffff: {
      map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return errno_to_sysret();
      }

      return sysret_s_ok(static_cast<uintptr_t>(get_cap_type(cap_slot->cap)));
    }
    case SYS_CAP_COPY & 0xffff: {
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
    case SYS_CAP_REVOKE & 0xffff: {
      map_ptr<cap_slot_t> cap_slot = lookup_cap(task, args->args[0]);
      if (cap_slot == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return errno_to_sysret();
      }

      if (revoke_cap(cap_slot)) {
        return sysret_s_ok(0);
      } else {
        loge(tag, "Failed to revoke cap: %d", args->args[0]);
        return errno_to_sysret();
      }
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_mem_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }
  if (get_cap_type(cap_slot->cap) != CAP_MEM) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_cap_type();
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
        return errno_to_sysret();
      }
      return sysret_s_ok(get_cap_slot_index(result));
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_task_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }
  if (get_cap_type(cap_slot->cap) != CAP_TASK) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_cap_type();
  }

  auto& task_cap = cap_slot->cap.task;

  switch (id) {
    case SYS_TASK_CAP_TID & 0xffff:
      return sysret_s_ok(std::bit_cast<uint32_t>(task_cap.task->tid));
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
        return sysret_e_cap_state();
      }
      kill_task(task_cap.task, static_cast<int>(args->args[1]));
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SWITCH & 0xffff:
      if (!task_cap.switchable) [[unlikely]] {
        loge(tag, "This task cap is not switchable: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      {
        std::lock_guard lock(task_cap.task->lock);

        if (task_cap.task->state != task_state_t::ready) {
          return sysret_e_ill_state();
        }

        remove_ready_queue(task_cap.task);
      }
      switch_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_SUSPEND & 0xffff:
      if (!task_cap.suspendable) [[unlikely]] {
        loge(tag, "This task cap is not suspendable: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      suspend_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_RESUME & 0xffff:
      if (!task_cap.resumable) [[unlikely]] {
        loge(tag, "This task cap is not resumable: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      resume_task(task_cap.task);
      return sysret_s_ok(0);
    case SYS_TASK_CAP_GET_REG & 0xffff:
      if (!task_cap.register_gettable) [[unlikely]] {
        loge(tag, "This task cap is not register gettable: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
        return sysret_e_ill_state();
      }
      if (args->args[1] > LAST_REGISTER) [[unlikely]] {
        loge(tag, "Invalid register index: %d", args->args[1]);
        return sysret_e_ill_args();
      }
      return sysret_s_ok(get_register(make_map_ptr(&task_cap.task->frame), args->args[1]));
    case SYS_TASK_CAP_SET_REG & 0xffff:
      if (!task_cap.register_settable) [[unlikely]] {
        loge(tag, "This task cap is not register settable: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
        return sysret_e_ill_state();
      }
      if (args->args[1] > LAST_REGISTER) [[unlikely]] {
        loge(tag, "Invalid register index: %d", args->args[1]);
        return sysret_e_ill_args();
      }
      return sysret_s_ok(set_register(make_map_ptr(&task_cap.task->frame), args->args[1], args->args[2]));
    case SYS_TASK_CAP_TRANSFER_CAP & 0xffff: {
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
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
    case SYS_TASK_CAP_DELEGATE_CAP & 0xffff: {
      if (task_cap.task->state != task_state_t::suspended) [[unlikely]] {
        loge(tag, "Unexpected task state: %d", task_cap.task->state);
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
    case SYS_TASK_CAP_GET_FREE_SLOT_COUNT & 0xffff:
      return sysret_s_ok(task_cap.task->free_slots_count);
    case SYS_TASK_CAP_GET_CAP_SPACE_COUNT & 0xffff:
      return sysret_s_ok(task_cap.task->cap_count.num_cap_space);
    case SYS_TASK_CAP_GET_CAP_SPACE_EXT_COUNT & 0xffff:
      return sysret_s_ok(task_cap.task->cap_count.num_extension);
    case SYS_TASK_CAP_INSERT_CAP_SPACE & 0xffff: {
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
    case SYS_TASK_CAP_EXTEND_CAP_SPACE & 0xffff: {
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
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_endpoint_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }
  if (get_cap_type(cap_slot->cap) != CAP_ENDPOINT) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_cap_type();
  }

  auto& ep_cap = cap_slot->cap.endpoint;

  switch (id) {
    case SYS_ENDPOINT_CAP_SEND_SHORT & 0xffff: {
      if (!ipc_send_short(true, ep_cap.endpoint, args->args[1], args->args[2], args->args[3], args->args[4], args->args[5], args->args[6])) {
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_SEND_LONG & 0xffff: {
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_to(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      if (!ipc_send_long(true, ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_RECEIVE & 0xffff: {
      if (!ipc_receive(true, ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_from(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_REPLY & 0xffff: {
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_to(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      if (!ipc_reply(ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_NB_SEND_SHORT & 0xffff:
      if (!ipc_send_short(false, ep_cap.endpoint, args->args[1], args->args[2], args->args[3], args->args[4], args->args[5], args->args[6])) {
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    case SYS_ENDPOINT_CAP_NB_SEND_LONG & 0xffff: {
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_to(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      if (!ipc_send_long(false, ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_NB_RECEIVE & 0xffff: {
      if (!ipc_receive(false, ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_from(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_CALL & 0xffff: {
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_to(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      if (!ipc_call(ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_from(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      return sysret_s_ok(0);
    }
    case SYS_ENDPOINT_CAP_REPLY_AND_RECEIVE & 0xffff: {
      bool copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_to(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      if (!ipc_reply(ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      if (!ipc_receive(true, ep_cap.endpoint)) {
        return errno_to_sysret();
      }
      copied = user_ptr<message_buffer_t>::from(get_cls()->current_task, args->args[1]).copy_from(make_map_ptr(&get_cls()->current_task->msg_buf));
      if (!copied) {
        loge(tag, "Failed to copy message buffer");
        return sysret_e_unknown();
      }
      return sysret_s_ok(0);
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_page_table_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }
  if (get_cap_type(cap_slot->cap) != CAP_PAGE_TABLE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_cap_type();
  }

  auto& page_table_cap = cap_slot->cap.page_table;

  switch (id) {
    case SYS_PAGE_TABLE_CAP_MAPPED & 0xffff:
      return sysret_s_ok(page_table_cap.mapped);
    case SYS_PAGE_TABLE_CAP_LEVEL & 0xffff:
      if (!page_table_cap.mapped) [[unlikely]] {
        loge(tag, "This page table cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(page_table_cap.level);
    case SYS_PAGE_TABLE_CAP_MAP_TABLE & 0xffff:
      if (!map_page_table_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to map page table: index=%d", args->args[1]);
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_UNMAP_TABLE & 0xffff:
      if (!unmap_page_table_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to unmap page table: index=%d", args->args[1]);
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_MAP_PAGE & 0xffff:
      if (!map_virt_page_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[5]), args->args[2], args->args[3], args->args[4])) [[unlikely]] {
        loge(tag, "Failed to map virt page: index=%d", args->args[1]);
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_UNMAP_PAGE & 0xffff:
      if (!unmap_virt_page_cap(cap_slot, args->args[1], lookup_cap(get_cls()->current_task, args->args[2]))) [[unlikely]] {
        loge(tag, "Failed to unmap virt page: index=%d", args->args[1]);
        return errno_to_sysret();
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
        return errno_to_sysret();
      }
      return sysret_s_ok(0);
    case SYS_PAGE_TABLE_CAP_VIRT_ADDR_BASE & 0xffff:
      if (!page_table_cap.mapped) [[unlikely]] {
        loge(tag, "This page table cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(page_table_cap.virt_addr_base);
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_virt_page_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<cap_slot_t> cap_slot = lookup_cap(get_cls()->current_task, args->args[0]);
  if (cap_slot == nullptr) [[unlikely]] {
    loge(tag, "Failed to look up cap: %d", args->args[0]);
    return errno_to_sysret();
  }
  if (get_cap_type(cap_slot->cap) != CAP_VIRT_PAGE) [[unlikely]] {
    loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot->cap));
    return sysret_e_cap_type();
  }

  auto& virt_page_cap = cap_slot->cap.virt_page;

  switch (id) {
    case SYS_VIRT_PAGE_CAP_MAPPED & 0xffff:
      return sysret_s_ok(virt_page_cap.mapped);
    case SYS_VIRT_PAGE_CAP_READABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.readable);
    case SYS_VIRT_PAGE_CAP_WRITABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.writable);
    case SYS_VIRT_PAGE_CAP_EXECUTABLE & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.executable);
    case SYS_VIRT_PAGE_CAP_LEVEL & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.level);
    case SYS_VIRT_PAGE_CAP_PHYS_ADDR & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.phys_addr);
    case SYS_VIRT_PAGE_CAP_VIRT_ADDR & 0xffff:
      if (!virt_page_cap.mapped) [[unlikely]] {
        loge(tag, "This virt page cap is not mapped: %d", args->args[0]);
        return sysret_e_cap_state();
      }
      return sysret_s_ok(virt_page_cap.address.raw());
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}

sysret_t invoke_syscall_id_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  map_ptr<task_t> task = get_cls()->current_task;
  std::lock_guard lock(task->lock);

  switch (id) {
    case SYS_ID_CAP_CREATE & 0xffff: {
      map_ptr<cap_slot_t> slot = pop_free_slots(task);
      if (slot == nullptr) [[unlikely]] {
        return sysret_e_out_of_cap_space();
      }

      map_ptr<cap_slot_t> result = create_id_object(slot);
      if (result == nullptr) [[unlikely]] {
        push_free_slots(task, slot);
        return errno_to_sysret();
      }

      return sysret_s_ok(get_cap_slot_index(result));
    }
    case SYS_ID_CAP_COMPARE & 0xffff: {
      map_ptr<cap_slot_t> cap_slot1 = lookup_cap(task, args->args[0]);
      if (cap_slot1 == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[0]);
        return errno_to_sysret();
      }
      if (get_cap_type(cap_slot1->cap) != CAP_ID) [[unlikely]] {
        loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot1->cap));
        return sysret_e_cap_type();
      }

      map_ptr<cap_slot_t> cap_slot2 = lookup_cap(task, args->args[1]);
      if (cap_slot2 == nullptr) [[unlikely]] {
        loge(tag, "Failed to look up cap: %d", args->args[1]);
        return errno_to_sysret();
      }
      if (get_cap_type(cap_slot2->cap) != CAP_ID) [[unlikely]] {
        loge(tag, "Invalid cap type: %d", get_cap_type(cap_slot2->cap));
        return sysret_e_cap_type();
      }

      return sysret_s_ok(static_cast<uintptr_t>(static_cast<intptr_t>(compare_id_cap(cap_slot1, cap_slot2))));
    }
    default:
      loge(tag, "Invalid syscall id: 0x%x", id);
      return sysret_e_ill_code();
  }
}
