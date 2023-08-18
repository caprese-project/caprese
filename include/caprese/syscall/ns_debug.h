#ifndef CAPRESE_SYSCALL_NS_DEBUG_H_
#define CAPRESE_SYSCALL_NS_DEBUG_H_

#include <caprese/syscall/handler.h>

namespace caprese::syscall::debug {
  constexpr uintptr_t NAMESPACE_ID    = 1;
  constexpr uintptr_t SYS_PUTCHAR_FID = 0;

  sysret_t sys_putchar(uintptr_t ch, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall::debug

#endif // CAPRESE_SYSCALL_NS_DEBUG_H_
