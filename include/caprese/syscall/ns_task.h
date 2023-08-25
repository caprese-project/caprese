#ifndef CAPRESE_SYSCALL_NS_TASK_H_
#define CAPRESE_SYSCALL_NS_TASK_H_

#include <caprese/syscall/handler.h>

namespace caprese::syscall::task {
  constexpr uintptr_t NAMESPACE_ID = 3;
  constexpr uintptr_t TID_FID      = 0;
  constexpr uintptr_t CREATE_FID   = 1;
  constexpr uintptr_t YIELD_FID    = 2;
  constexpr uintptr_t IPC_RECEIVE  = 3;

  sysret_t sys_tid([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_create([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_yield([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);
  sysret_t sys_ipc_receive(uintptr_t msg_addr, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t);

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::syscall::task

#endif // CAPRESE_SYSCALL_NS_TASK_H_
