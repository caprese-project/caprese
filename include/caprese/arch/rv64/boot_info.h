#ifndef CAPRESE_ARCH_RV64_BOOT_INFO_H_
#define CAPRESE_ARCH_RV64_BOOT_INFO_H_

#include <cstdint>

namespace caprese::arch::inline rv64 {
  struct boot_info_t {
    uint64_t hartid;
    char*    device_tree_blob;
  };
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_BOOT_INFO_H_
