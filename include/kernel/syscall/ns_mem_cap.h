#ifndef KERNEL_SYSCALL_NS_MEM_CAP_H_
#define KERNEL_SYSCALL_NS_MEM_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_mem_cap_device(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_mem_cap_phys_addr(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_mem_cap_size(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_mem_cap_used_size(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_mem_cap_create_object(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_mem_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_MEM_CAP_DEVICE & 0xffff]        = invoke_sys_mem_cap_device,
  [SYS_MEM_CAP_PHYS_ADDR & 0xffff]     = invoke_sys_mem_cap_phys_addr,
  [SYS_MEM_CAP_SIZE & 0xffff]          = invoke_sys_mem_cap_size,
  [SYS_MEM_CAP_USED_SIZE & 0xffff]     = invoke_sys_mem_cap_used_size,
  [SYS_MEM_CAP_CREATE_OBJECT & 0xffff] = invoke_sys_mem_cap_create_object,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_MEM_CAP_H_
