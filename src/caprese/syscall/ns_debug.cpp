#include <cstdio>

#include <caprese/syscall/ns_debug.h>
#include <caprese/util/array_size.h>

namespace caprese::syscall::debug {
  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [SYS_PUTCHAR_FID] = sys_putchar,
    };
  } // namespace

  sysret_t sys_putchar(uintptr_t ch, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    int res = putchar(ch);
    return { .result = static_cast<uintptr_t>(res), .error = 0 };
  }

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (function_id >= array_size_of(handler_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

#ifdef NDEBUG
    return { .result = 0, .error = 1 };
#else
    return handler_table[function_id](arg0, arg1, arg2, arg3, arg4, arg5);
#endif
  }
} // namespace caprese::syscall::debug
