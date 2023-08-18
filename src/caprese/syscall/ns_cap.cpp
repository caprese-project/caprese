#include <caprese/capability/capability.h>
#include <caprese/syscall/ns_cap.h>
#include <caprese/task/task.h>
#include <caprese/util/array_size.h>

namespace caprese::syscall::cap {
  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [CREATE_CLASS_FID] = sys_create_class, [CREATE_FID] = sys_create,       [CALL_METHOD_FID] = sys_call_method, [GET_FIELD_FID] = sys_get_field,
      [IS_PERMITTED_FID] = sys_is_permitted, [LIST_SIZE_FID] = sys_list_size,
    };
  } // namespace

  sysret_t sys_create_class([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    // TODO: impl
    return { .result = 0, .error = 1 };
  }

  sysret_t sys_create([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    // TODO: impl
    return { .result = 0, .error = 1 };
  }

  sysret_t sys_call_method(uintptr_t cid, uintptr_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
    task::task_t*             task       = task::get_current_task();
    capability::capability_t* capability = task::lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));

    if (capability == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::cap_ret_t ret = capability::call_method(capability, method, arg0, arg1, arg2, arg3);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_get_field(uintptr_t cid, uintptr_t field, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t*             task       = task::get_current_task();
    capability::capability_t* capability = task::lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));
    if (capability == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    capability::cap_ret_t ret = capability::get_field(capability, field);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_is_permitted(uintptr_t cid, uintptr_t permission, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t*             task       = task::get_current_task();
    capability::capability_t* capability = task::lookup_capability(task, std::bit_cast<capability::cid_t>(static_cast<uint64_t>(cid)));
    if (capability == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    capability::cap_ret_t ret = capability::is_permitted(capability, permission);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_list_size([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t* task = task::get_current_task();
    return { .result = task::allocated_cap_list_size(task), .error = 0 };
  }

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (function_id >= array_size_of(handler_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    return handler_table[function_id](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::syscall::cap
