#ifndef CAPRESE_ARCH_RV64_SYSCALL_H_
#define CAPRESE_ARCH_RV64_SYSCALL_H_

#include <cstdint>

namespace caprese::arch::inline rv64 {
  namespace syscall { }

  uintptr_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_SYSCALL_H_
