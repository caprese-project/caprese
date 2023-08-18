#ifndef CAPRESE_SYSCALL_NS_CAP_H_
#define CAPRESE_SYSCALL_NS_CAP_H_

#include <caprese/syscall/handler.h>

namespace caprese::syscall::cap {
  constexpr uintptr_t NAMESPACE_ID     = 2;
  constexpr uintptr_t CREATE_CLASS_FID = 0;
  constexpr uintptr_t CREATE_FID       = 1;
  constexpr uintptr_t CALL_METHOD_FID  = 2;
  constexpr uintptr_t GET_FIELD_FID    = 3;
  constexpr uintptr_t IS_PERMITTED_FID = 4;
  constexpr uintptr_t LIST_SIZE_FID    = 5;

  sysret_t sys_create_class([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_create([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_call_method(uintptr_t cid, uintptr_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
  sysret_t sys_get_field(uintptr_t cid, uintptr_t field, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_is_permitted(uintptr_t cid, uintptr_t permission, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_list_size([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall::cap

#endif // CAPRESE_SYSCALL_NS_CAP_H_
