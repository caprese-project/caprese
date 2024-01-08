#ifndef KERNEL_SYSCALL_NS_ID_CAP_H_
#define KERNEL_SYSCALL_NS_ID_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_id_cap_create(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_id_cap_compare(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_id_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_ID_CAP_CREATE & 0xffff]  = invoke_sys_id_cap_create,
  [SYS_ID_CAP_COMPARE & 0xffff] = invoke_sys_id_cap_compare,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_ID_CAP_H_
