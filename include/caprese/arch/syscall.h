#ifndef CAPRESE_ARCH_SYSCALL_H_
#define CAPRESE_ARCH_SYSCALL_H_

#if defined(CONFIG_ARCH_RISCV) && defined(CONFIG_XLEN_64)
#include <caprese/arch/rv64/syscall.h>
#endif

#endif // CAPRESE_ARCH_SYSCALL_H_
