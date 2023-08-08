#ifndef CAPRESE_TASK_SYSCALL_H_
#define CAPRESE_TASK_SYSCALL_H_

#include <cstdint>

namespace caprese::task {
  void handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::task

#endif // CAPRESE_TASK_SYSCALL_H_
