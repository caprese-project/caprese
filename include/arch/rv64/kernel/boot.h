#ifndef ARCH_RV64_KERNEL_BOOT_H_
#define ARCH_RV64_KERNEL_BOOT_H_

#include <cstdint>

#include <kernel/address.h>
#include <kernel/core_id.h>

struct boot_info_t {
  core_id_t     core_id;
  size_t        cap_count;
  map_ptr<char> dtb;
};

#endif // ARCH_RV64_KERNEL_BOOT_H_
