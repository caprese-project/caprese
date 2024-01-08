#ifndef KERNEL_SYSCALL_NS_ENDPOINT_CAP_H_
#define KERNEL_SYSCALL_NS_ENDPOINT_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_endpoint_cap_send_short(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_send_long(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_receive(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_reply(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_nb_send_short(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_nb_send_long(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_nb_receive(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_call(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_endpoint_cap_reply_and_receive(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_endpoint_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_ENDPOINT_CAP_SEND_SHORT & 0xffff]        = invoke_sys_endpoint_cap_send_short,
  [SYS_ENDPOINT_CAP_SEND_LONG & 0xffff]         = invoke_sys_endpoint_cap_send_long,
  [SYS_ENDPOINT_CAP_RECEIVE & 0xffff]           = invoke_sys_endpoint_cap_receive,
  [SYS_ENDPOINT_CAP_REPLY & 0xffff]             = invoke_sys_endpoint_cap_reply,
  [SYS_ENDPOINT_CAP_NB_SEND_SHORT & 0xffff]     = invoke_sys_endpoint_cap_nb_send_short,
  [SYS_ENDPOINT_CAP_NB_SEND_LONG & 0xffff]      = invoke_sys_endpoint_cap_nb_send_long,
  [SYS_ENDPOINT_CAP_NB_RECEIVE & 0xffff]        = invoke_sys_endpoint_cap_nb_receive,
  [SYS_ENDPOINT_CAP_CALL & 0xffff]              = invoke_sys_endpoint_cap_call,
  [SYS_ENDPOINT_CAP_REPLY_AND_RECEIVE & 0xffff] = invoke_sys_endpoint_cap_reply_and_receive,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_ENDPOINT_CAP_H_
