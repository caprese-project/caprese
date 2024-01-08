#ifndef KERNEL_SYSCALL_NS_PAGE_TABLE_CAP_H_
#define KERNEL_SYSCALL_NS_PAGE_TABLE_CAP_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_page_table_cap_mapped(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_level(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_map_table(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_unmap_table(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_map_page(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_unmap_page(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_remap_page(map_ptr<syscall_args_t> args);
sysret_t invoke_sys_page_table_cap_virt_addr_base(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_page_table_cap_table[])(map_ptr<syscall_args_t>) = {
  [SYS_PAGE_TABLE_CAP_MAPPED & 0xffff]         = invoke_sys_page_table_cap_mapped,
  [SYS_PAGE_TABLE_CAP_LEVEL & 0xffff]          = invoke_sys_page_table_cap_level,
  [SYS_PAGE_TABLE_CAP_MAP_TABLE & 0xffff]      = invoke_sys_page_table_cap_map_table,
  [SYS_PAGE_TABLE_CAP_UNMAP_TABLE & 0xffff]    = invoke_sys_page_table_cap_unmap_table,
  [SYS_PAGE_TABLE_CAP_MAP_PAGE & 0xffff]       = invoke_sys_page_table_cap_map_page,
  [SYS_PAGE_TABLE_CAP_UNMAP_PAGE & 0xffff]     = invoke_sys_page_table_cap_unmap_page,
  [SYS_PAGE_TABLE_CAP_REMAP_PAGE & 0xffff]     = invoke_sys_page_table_cap_remap_page,
  [SYS_PAGE_TABLE_CAP_VIRT_ADDR_BASE & 0xffff] = invoke_sys_page_table_cap_virt_addr_base,
};

// clang-format on

#endif // KERNEL_SYSCALL_NS_PAGE_TABLE_CAP_H_
