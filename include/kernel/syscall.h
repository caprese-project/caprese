#ifndef KERNEL_SYSCALL_H_
#define KERNEL_SYSCALL_H_

#include <kernel/task.h>
#include <libcaprese/syscall.h>

struct syscall_args_t {
  uintptr_t args[7];
  uintptr_t code;
};

void get_syscall_args(map_ptr<syscall_args_t> args);

constexpr sysret_t sysret_s_ok(uintptr_t result) {
  return sysret_t { result, SYS_S_OK };
}

constexpr sysret_t sysret_e_invalid_argument() {
  return sysret_t { 0, SYS_E_INVALID_ARGUMENT };
}

constexpr sysret_t sysret_e_invalid_code() {
  return sysret_t { 0, SYS_E_INVALID_CODE };
}

sysret_t invoke_syscall();
sysret_t invoke_syscall_system(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_arch(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_mem_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_task_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_endpoint_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_page_table_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_virt_page_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_id_cap(uint16_t id, map_ptr<syscall_args_t> args);

#endif // KERNEL_SYSCALL_H_
