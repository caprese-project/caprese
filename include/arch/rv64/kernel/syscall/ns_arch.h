#ifndef ARCH_RV64_KERNEL_SYSCALL_NS_ARCH_H_
#define ARCH_RV64_KERNEL_SYSCALL_NS_ARCH_H_

#include <kernel/address.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

sysret_t invoke_sys_arch_mmu_mode(map_ptr<syscall_args_t> args);

// clang-format off

constexpr sysret_t (*const sysns_arch_table[])(map_ptr<syscall_args_t>) = {
  [SYS_ARCH_MMU_MODE & 0xffff] = invoke_sys_arch_mmu_mode,
};

// clang-format on

#endif // ARCH_RV64_KERNEL_SYSCALL_NS_ARCH_H_
