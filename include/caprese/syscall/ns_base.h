#ifndef CAPRESE_SYSCALL_NS_BASE_H_
#define CAPRESE_SYSCALL_NS_BASE_H_

#include <caprese/syscall/handler.h>

namespace caprese::syscall::base {
  constexpr uintptr_t NAMESPACE_ID    = 0;
  constexpr uintptr_t SYS_NULL_FID    = 0;
  constexpr uintptr_t SYS_CORE_ID_FID = 1;

  sysret_t sys_null([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_core_id([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall::base

#endif // CAPRESE_SYSCALL_NS_BASE_H_