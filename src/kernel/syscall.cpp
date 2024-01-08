#include <iterator>

#include <kernel/log.h>
#include <kernel/syscall.h>
#include <kernel/syscall/ns_arch.h>
#include <kernel/syscall/ns_cap.h>
#include <kernel/syscall/ns_endpoint_cap.h>
#include <kernel/syscall/ns_id_cap.h>
#include <kernel/syscall/ns_mem_cap.h>
#include <kernel/syscall/ns_page_table_cap.h>
#include <kernel/syscall/ns_system.h>
#include <kernel/syscall/ns_task_cap.h>
#include <kernel/syscall/ns_virt_page_cap.h>

namespace {
  constexpr const char* tag = "kernel/syscall";

  constexpr sysret_t (*const ns_table[])(uint16_t, map_ptr<syscall_args_t>) = {
    [SYSNS_SYSTEM >> 16]         = invoke_syscall_system,
    [SYSNS_ARCH >> 16]           = invoke_syscall_arch,
    [SYSNS_CAP >> 16]            = invoke_syscall_cap,
    [SYSNS_MEM_CAP >> 16]        = invoke_syscall_mem_cap,
    [SYSNS_TASK_CAP >> 16]       = invoke_syscall_task_cap,
    [SYSNS_ENDPOINT_CAP >> 16]   = invoke_syscall_endpoint_cap,
    [SYSNS_PAGE_TABLE_CAP >> 16] = invoke_syscall_page_table_cap,
    [SYSNS_VIRT_PAGE_CAP >> 16]  = invoke_syscall_virt_page_cap,
    [SYSNS_ID_CAP >> 16]         = invoke_syscall_id_cap,
  };
} // namespace

sysret_t invoke_syscall() {
  syscall_args_t args;
  get_syscall_args(make_map_ptr(&args));

  size_t ns = (args.code >> 16) & 0xffff;

  if (ns >= std::size(ns_table)) [[unlikely]] {
    loge(tag, "Invalid syscall namespace: 0x%x", ns);
    return sysret_e_ill_code();
  }

  uint16_t id = static_cast<uint16_t>(args.code & 0xffff);

  return ns_table[ns](id, make_map_ptr(&args));
}

sysret_t invoke_syscall_system(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_system_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_system_table[id](args);
}

sysret_t invoke_syscall_arch(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_arch_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_arch_table[id](args);
}

sysret_t invoke_syscall_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_cap_table[id](args);
}

sysret_t invoke_syscall_mem_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_mem_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_mem_cap_table[id](args);
}

sysret_t invoke_syscall_task_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_task_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_task_cap_table[id](args);
}

sysret_t invoke_syscall_endpoint_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_endpoint_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_endpoint_cap_table[id](args);
}

sysret_t invoke_syscall_page_table_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_page_table_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_page_table_cap_table[id](args);
}

sysret_t invoke_syscall_virt_page_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_virt_page_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_virt_page_cap_table[id](args);
}

sysret_t invoke_syscall_id_cap(uint16_t id, map_ptr<syscall_args_t> args) {
  if (id >= std::size(sysns_id_cap_table)) [[unlikely]] {
    loge(tag, "Invalid syscall id: 0x%x", id);
    return sysret_e_ill_code();
  }

  return sysns_id_cap_table[id](args);
}
