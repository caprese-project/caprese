#ifndef CAPRESE_SYSCALL_HANDLER_H_
#define CAPRESE_SYSCALL_HANDLER_H_

#include <cstdint>

namespace caprese::syscall {
  struct sysret_t {
    uintptr_t result;
    uintptr_t error;
  };

  [[nodiscard]] sysret_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall

#endif // CAPRESE_SYSCALL_HANDLER_H_
