#include <caprese/memory/cls.h>
#include <caprese/syscall/ns_base.h>
#include <caprese/util/array_size.h>

namespace caprese::syscall::base {
  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [SYS_NULL_FID]    = sys_null,
      [SYS_CORE_ID_FID] = sys_core_id,
    };
  } // namespace

  sysret_t sys_null([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    return { .result = 0, .error = 0 };
  }

  sysret_t sys_core_id([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    return { .result = memory::get_cls()->core_id, .error = 0 };
  }

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (function_id >= array_size_of(handler_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    return handler_table[function_id](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::syscall::base
