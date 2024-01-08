#ifndef KERNEL_SYSCALL_NS_TASK_CAP_H_
#define KERNEL_SYSCALL_NS_TASK_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_task_cap_tid(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_killable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_switchable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_suspendable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_resumable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_kill(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_switch(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_suspend(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_resume(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_get_reg(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_set_reg(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_transfer_cap(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_delegate_cap(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_get_free_slot_count(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_get_cap_space_count(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_get_cap_space_ext_count(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_insert_cap_space(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_extend_cap_space(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_task_cap_set_kill_notify(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_task_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_TASK_CAP_TID & 0xffff]                     = invoke_sys_task_cap_tid,
  [SYS_TASK_CAP_KILLABLE & 0xffff]                = invoke_sys_task_cap_killable,
  [SYS_TASK_CAP_SWITCHABLE & 0xffff]              = invoke_sys_task_cap_switchable,
  [SYS_TASK_CAP_SUSPENDABLE & 0xffff]             = invoke_sys_task_cap_suspendable,
  [SYS_TASK_CAP_RESUMABLE & 0xffff]               = invoke_sys_task_cap_resumable,
  [SYS_TASK_CAP_KILL & 0xffff]                    = invoke_sys_task_cap_kill,
  [SYS_TASK_CAP_SWITCH & 0xffff]                  = invoke_sys_task_cap_switch,
  [SYS_TASK_CAP_SUSPEND & 0xffff]                 = invoke_sys_task_cap_suspend,
  [SYS_TASK_CAP_RESUME & 0xffff]                  = invoke_sys_task_cap_resume,
  [SYS_TASK_CAP_GET_REG & 0xffff]                 = invoke_sys_task_cap_get_reg,
  [SYS_TASK_CAP_SET_REG & 0xffff]                 = invoke_sys_task_cap_set_reg,
  [SYS_TASK_CAP_TRANSFER_CAP & 0xffff]            = invoke_sys_task_cap_transfer_cap,
  [SYS_TASK_CAP_DELEGATE_CAP & 0xffff]            = invoke_sys_task_cap_delegate_cap,
  [SYS_TASK_CAP_GET_FREE_SLOT_COUNT & 0xffff]     = invoke_sys_task_cap_get_free_slot_count,
  [SYS_TASK_CAP_GET_CAP_SPACE_COUNT & 0xffff]     = invoke_sys_task_cap_get_cap_space_count,
  [SYS_TASK_CAP_GET_CAP_SPACE_EXT_COUNT & 0xffff] = invoke_sys_task_cap_get_cap_space_ext_count,
  [SYS_TASK_CAP_INSERT_CAP_SPACE & 0xffff]        = invoke_sys_task_cap_insert_cap_space,
  [SYS_TASK_CAP_EXTEND_CAP_SPACE & 0xffff]        = invoke_sys_task_cap_extend_cap_space,
  [SYS_TASK_CAP_SET_KILL_NOTIFY & 0xffff]         = invoke_sys_task_cap_set_kill_notify,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_TASK_CAP_H_
