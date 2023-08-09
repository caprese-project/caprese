#include <bit>
#include <cstdio>

#include <caprese/arch/syscall.h>
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

#if !defined(NDEBUG)
    uintptr_t debug_putchar(uintptr_t                  arg0,
                            [[maybe_unused]] uintptr_t arg1,
                            [[maybe_unused]] uintptr_t arg2,
                            [[maybe_unused]] uintptr_t arg3,
                            [[maybe_unused]] uintptr_t arg4,
                            [[maybe_unused]] uintptr_t arg5) {
      putchar(arg0);
      return 0;
    }
#endif // !defined(NDEBUG)

    namespace {
      using syscall_handler_t                 = uintptr_t (*)(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
      const syscall_handler_t syscall_table[] = {
        null,
#if !defined(NDEBUG)
        debug_putchar,
#endif // !defined(NDEBUG)
      };
    }; // namespace
  }    // namespace syscall

  uintptr_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    constexpr uintptr_t mask = static_cast<uintptr_t>(1) << (sizeof(uintptr_t) * 8 - 1);
    if (code & mask) {
      return arch::handle_system_call(code & ~mask, arg0, arg1, arg2, arg3, arg4, arg5);
    }
    if (code >= array_size_of(syscall::syscall_table)) [[unlikely]] {
      return -1;
    }
    return syscall::syscall_table[code](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::task
