#ifndef ARCH_RV64_KERNEL_BOOT_H_
#define ARCH_RV64_KERNEL_BOOT_H_

#include <cstdint>

#include <kernel/address.h>

struct boot_info_t {
  uintptr_t  hartid;
  map_addr_t dtb;
};

#endif // ARCH_RV64_KERNEL_BOOT_H_
