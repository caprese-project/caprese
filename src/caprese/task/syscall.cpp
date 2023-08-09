#include <bit>
#include <cstdio>

#include <caprese/task/syscall.h>
#include <caprese/util/array_size.h>

namespace caprese::task {
  namespace syscall {
    uintptr_t null([[maybe_unused]] uintptr_t arg0,
                   [[maybe_unused]] uintptr_t arg1,
                   [[maybe_unused]] uintptr_t arg2,
                   [[maybe_unused]] uintptr_t arg3,
                   [[maybe_unused]] uintptr_t arg4,
                   [[maybe_unused]] uintptr_t arg5) {
      return 0;
    }

    uintptr_t debug_putc(uintptr_t                  arg0,
                         [[maybe_unused]] uintptr_t arg1,
                         [[maybe_unused]] uintptr_t arg2,
                         [[maybe_unused]] uintptr_t arg3,
                         [[maybe_unused]] uintptr_t arg4,
                         [[maybe_unused]] uintptr_t arg5) {
      putchar(arg0);
      return 0;
    }

    namespace {
      using syscall_handler_t                 = uintptr_t (*)(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
      const syscall_handler_t syscall_table[] = { null, debug_putc };
    }; // namespace
  }    // namespace syscall

  uintptr_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (code >= array_size_of(syscall::syscall_table)) [[unlikely]] {
      return -1;
    }
    return syscall::syscall_table[code](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::task
