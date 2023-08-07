#ifndef CAPRESE_ARCH_RV64_SYSTEM_H_
#define CAPRESE_ARCH_RV64_SYSTEM_H_

#include <cstddef>

namespace caprese::arch::inline rv64 {
  size_t get_core_id();
  void dump_system_context();
}

#endif // CAPRESE_ARCH_RV64_SYSTEM_H_
