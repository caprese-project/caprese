#ifndef CAPRESE_ARCH_RV64_SYSTEM_H_
#define CAPRESE_ARCH_RV64_SYSTEM_H_

#include <cstddef>

namespace caprese::arch::inline rv64 {
  [[nodiscard]] size_t get_core_id();
  void                 dump_system_context();
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_SYSTEM_H_
