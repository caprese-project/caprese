#ifndef INIT_ARCH_BOOT_H_
#define INIT_ARCH_BOOT_H_

#if defined(CONFIG_ARCH_RISCV) && defined(CONFIG_XLEN_64)
#include <init/arch/rv64/boot.h>
#endif

#endif // INIT_ARCH_BOOT_H_
