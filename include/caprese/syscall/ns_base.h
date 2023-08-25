#ifndef CAPRESE_SYSCALL_NS_BASE_H_
#define CAPRESE_SYSCALL_NS_BASE_H_

#include <caprese/syscall/handler.h>

namespace caprese::syscall::base {
  constexpr uintptr_t NAMESPACE_ID         = 0;
  constexpr uintptr_t NULL_FID             = 0;
  constexpr uintptr_t CORE_ID_FID          = 1;
  constexpr uintptr_t PAGE_SIZE_FID        = 2;
  constexpr uintptr_t USER_SPACE_START_FID = 3;
  constexpr uintptr_t USER_SPACE_END_FID   = 4;

  sysret_t sys_null([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_core_id([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_page_size([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_user_space_start([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_user_space_end([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall::base

#endif // CAPRESE_SYSCALL_NS_BASE_H_
