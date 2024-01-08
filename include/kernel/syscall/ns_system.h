#ifndef KERNEL_SYSCALL_NS_SYSTEM_H_
#define KERNEL_SYSCALL_NS_SYSTEM_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_system_null(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_core_id(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_page_size(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_user_space_start(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_user_space_end(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_caps_per_cap_space(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_yield(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_cap_size(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_system_cap_align(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_system_table[])(map_ptr<syscall_args_t>) = {
  [SYS_SYSTEM_NULL & 0xffff]               = invoke_sys_system_null,
  [SYS_SYSTEM_CORE_ID & 0xffff]            = invoke_sys_system_core_id,
  [SYS_SYSTEM_PAGE_SIZE & 0xffff]          = invoke_sys_system_page_size,
  [SYS_SYSTEM_USER_SPACE_START & 0xffff]   = invoke_sys_system_user_space_start,
  [SYS_SYSTEM_USER_SPACE_END & 0xffff]     = invoke_sys_system_user_space_end,
  [SYS_SYSTEM_CAPS_PER_CAP_SPACE & 0xffff] = invoke_sys_system_caps_per_cap_space,
  [SYS_SYSTEM_YIELD & 0xffff]              = invoke_sys_system_yield,
  [SYS_SYSTEM_CAP_SIZE & 0xffff]           = invoke_sys_system_cap_size,
  [SYS_SYSTEM_CAP_ALIGN & 0xffff]          = invoke_sys_system_cap_align,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_SYSTEM_H_
