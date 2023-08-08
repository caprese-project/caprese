#include <cstdio>

#include <caprese/task/syscall.h>

namespace caprese::task {
  void handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    printf("syscall stub: code=0x%lx, arg0=0x%lx, arg1=0x%lx, arg2=0x%lx, arg3=0x%lx, arg4=0x%lx, arg5=0x%lx\n", code, arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::task
