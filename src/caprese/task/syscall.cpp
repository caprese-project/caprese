#include <bit>
#include <cstdio>

#include <caprese/task/syscall.h>
#include <caprese/task/task.h>
#include <caprese/util/array_size.h>

namespace caprese::task {
  namespace syscall {
    sysret_t null([[maybe_unused]] uintptr_t arg0,
                  [[maybe_unused]] uintptr_t arg1,
                  [[maybe_unused]] uintptr_t arg2,
                  [[maybe_unused]] uintptr_t arg3,
                  [[maybe_unused]] uintptr_t arg4,
                  [[maybe_unused]] uintptr_t arg5) {
      return { .result = 0, .error = 0 };
    }

    sysret_t debug_putchar(
        uintptr_t ch, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3, [[maybe_unused]] uintptr_t arg4, [[maybe_unused]] uintptr_t arg5) {
#if !defined(NDEBUG)
      putchar(ch);
#endif // !defined(NDEBUG)
      return { .result = 0, .error = 0 };
    }

    sysret_t cap_create_class([[maybe_unused]] uintptr_t arg0,
                              [[maybe_unused]] uintptr_t arg1,
                              [[maybe_unused]] uintptr_t arg2,
                              [[maybe_unused]] uintptr_t arg3,
                              [[maybe_unused]] uintptr_t arg4,
                              [[maybe_unused]] uintptr_t arg5) {
      // TODO: impl
      return { .result = 0, .error = 1 };
    }

    sysret_t cap_create([[maybe_unused]] uintptr_t ccid,
                        [[maybe_unused]] uintptr_t arg1,
                        [[maybe_unused]] uintptr_t arg2,
                        [[maybe_unused]] uintptr_t arg3,
                        [[maybe_unused]] uintptr_t arg4,
                        [[maybe_unused]] uintptr_t arg5) {
      // TODO: impl
      return { .result = 0, .error = 1 };
    }

    sysret_t cap_call_method(uintptr_t cid, uintptr_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
      task_t*                   task       = get_current_task();
      capability::capability_t* capability = lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));

      if (capability == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capability::cap_ret_t ret = capability::call_method(capability, method, arg0, arg1, arg2, arg3);
      return { .result = ret.result, .error = ret.error };
    }

    sysret_t cap_get_field(uintptr_t cid, uintptr_t field, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3, [[maybe_unused]] uintptr_t arg4, [[maybe_unused]] uintptr_t arg5) {
      task_t*                   task       = get_current_task();
      capability::capability_t* capability = lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));
      if (capability == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      capability::cap_ret_t ret = capability::get_field(capability, field);
      return { .result = ret.result, .error = ret.error };
    }

    sysret_t cap_is_permitted(uintptr_t cid, uintptr_t permission, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3, [[maybe_unused]] uintptr_t arg4, [[maybe_unused]] uintptr_t arg5) {
      task_t*                   task       = get_current_task();
      capability::capability_t* capability = lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));
      if (capability == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      capability::cap_ret_t ret = capability::is_permitted(capability, permission);
      return { .result = ret.result, .error = ret.error };
    }

    sysret_t cap_list_size([[maybe_unused]] uintptr_t arg0,
                           [[maybe_unused]] uintptr_t arg1,
                           [[maybe_unused]] uintptr_t arg2,
                           [[maybe_unused]] uintptr_t arg3,
                           [[maybe_unused]] uintptr_t arg4,
                           [[maybe_unused]] uintptr_t arg5) {
      task_t* task = get_current_task();
      return { .result = allocated_cap_list_size(task), .error = 0 };
    }

    namespace {
      using syscall_handler_t                 = sysret_t (*)(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
      const syscall_handler_t syscall_table[] = {
        [SYS_NULL]             = null,
        [SYS_DEBUG_PUTCHAR]    = debug_putchar,
        [SYS_CAP_CREATE_CLASS] = cap_create_class,
        [SYS_CAP_CREATE]       = cap_create,
        [SYS_CAP_CALL_METHOD]  = cap_call_method,
        [SYS_CAP_GET_FIELD]    = cap_get_field,
        [SYS_CAP_IS_PERMITTED] = cap_is_permitted,
        [SYS_CAP_LIST_SIZE]    = cap_list_size,
      };
    }; // namespace
  }    // namespace syscall

  sysret_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (code >= array_size_of(syscall::syscall_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    return syscall::syscall_table[code](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::task
