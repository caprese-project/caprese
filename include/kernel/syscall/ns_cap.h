#ifndef KERNEL_SYSCALL_NS_CAP_H_
#define KERNEL_SYSCALL_NS_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_cap_type(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_cap_copy(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_cap_revoke(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_cap_destroy(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_cap_same(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_CAP_TYPE & 0xffff]    = invoke_sys_cap_type,
  [SYS_CAP_COPY & 0xffff]    = invoke_sys_cap_copy,
  [SYS_CAP_REVOKE & 0xffff]  = invoke_sys_cap_revoke,
  [SYS_CAP_DESTROY & 0xffff] = invoke_sys_cap_destroy,
  [SYS_CAP_SAME & 0xffff]    = invoke_sys_cap_same,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_CAP_H_
