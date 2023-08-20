#include <caprese/syscall/handler.h>
#include <caprese/syscall/ns_base.h>
#include <caprese/syscall/ns_cap.h>
#include <caprese/syscall/ns_debug.h>
#include <caprese/syscall/ns_task.h>
#include <caprese/util/array.h>

namespace caprese::syscall {
  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [base::NAMESPACE_ID]  = base::handle_system_call,
      [debug::NAMESPACE_ID] = debug::handle_system_call,
      [cap::NAMESPACE_ID]   = cap::handle_system_call,
      [task::NAMESPACE_ID]  = task::handle_system_call,
    };
  } // namespace

  sysret_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    uintptr_t namespace_id = code >> 12;
    uintptr_t function_id  = code & 0xfff;

    if (namespace_id >= array_size_of(handler_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    return handler_table[namespace_id](function_id, arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::syscall
