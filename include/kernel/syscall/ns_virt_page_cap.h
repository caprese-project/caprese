#ifndef KERNEL_SYSCALL_NS_VIRT_PAGE_CAP_H_
#define KERNEL_SYSCALL_NS_VIRT_PAGE_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_virt_page_cap_mapped(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_readable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_writable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_executable(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_level(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_phys_addr(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_virt_page_cap_virt_addr(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_virt_page_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_VIRT_PAGE_CAP_MAPPED & 0xffff]     = invoke_sys_virt_page_cap_mapped,
  [SYS_VIRT_PAGE_CAP_READABLE & 0xffff]   = invoke_sys_virt_page_cap_readable,
  [SYS_VIRT_PAGE_CAP_WRITABLE & 0xffff]   = invoke_sys_virt_page_cap_writable,
  [SYS_VIRT_PAGE_CAP_EXECUTABLE & 0xffff] = invoke_sys_virt_page_cap_executable,
  [SYS_VIRT_PAGE_CAP_LEVEL & 0xffff]      = invoke_sys_virt_page_cap_level,
  [SYS_VIRT_PAGE_CAP_PHYS_ADDR & 0xffff]  = invoke_sys_virt_page_cap_phys_addr,
  [SYS_VIRT_PAGE_CAP_VIRT_ADDR & 0xffff]  = invoke_sys_virt_page_cap_virt_addr,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_VIRT_PAGE_CAP_H_
