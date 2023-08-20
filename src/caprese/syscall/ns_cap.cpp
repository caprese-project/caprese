#include <caprese/arch/builtin_capability.h>
#include <caprese/capability/bic/task.h>
#include <caprese/capability/capability.h>
#include <caprese/syscall/ns_cap.h>
#include <caprese/task/task.h>
#include <caprese/util/array.h>

namespace caprese::syscall::cap {
  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [CREATE_CLASS_FID] = sys_create_class, [CREATE_FID] = sys_create,       [CALL_METHOD_FID] = sys_call_method, [GET_FIELD_FID] = sys_get_field,
      [IS_PERMITTED_FID] = sys_is_permitted, [LIST_BASE_FID] = sys_list_base, [LIST_SIZE_FID] = sys_list_size,     [MOVE_FID] = sys_move,
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

  sysret_t sys_call_method(uintptr_t cid_handle, uintptr_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
    task::task_t* task = task::get_current_task();

    task::cid_t* cid = task::lookup_cid(task, cid_handle);
    if (cid == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::capability_t* cap = task::lookup_capability(task, *cid);
    if (cap == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::cap_ret_t ret = capability::call_method(cap, method, arg0, arg1, arg2, arg3);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_get_field(uintptr_t cid_handle, uintptr_t field, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t* task = task::get_current_task();

    task::cid_t* cid = task::lookup_cid(task, cid_handle);
    if (cid == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::capability_t* cap = task::lookup_capability(task, *cid);
    if (cap == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::cap_ret_t ret = capability::get_field(cap, field);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_is_permitted(uintptr_t cid_handle, uintptr_t permission, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t* task = task::get_current_task();

    task::cid_t* cid = task::lookup_cid(task, cid_handle);
    if (cid == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::capability_t* cap = task::lookup_capability(task, *cid);
    if (cap == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::cap_ret_t ret = capability::is_permitted(cap, permission);
    return { .result = ret.result, .error = ret.error };
  }

  sysret_t sys_list_base([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    return { .result = CONFIG_CAPABILITY_LIST_SPACE_BASE, .error = 0 };
  }

  sysret_t sys_list_size([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t* task = task::get_current_task();
    return { .result = task::allocated_cap_list_size(task), .error = 0 };
  }

  sysret_t sys_move(uintptr_t cid_handle, uintptr_t dst_task_cid_handle, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task::task_t* task = task::get_current_task();

    task::cid_t* dst_task_cid = task::lookup_cid(task, dst_task_cid_handle);
    if (dst_task_cid == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::capability_t* dst_task_cap = task::lookup_capability(task, *dst_task_cid);
    if (dst_task_cap == nullptr || dst_task_cap->ccid != capability::bic::task::CCID) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [dst_task_tid, dst_task_tid_error] = capability::get_field(dst_task_cap, capability::bic::task::field::TID);
    if (dst_task_tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task::task_t* dst_task = task::lookup(std::bit_cast<task::tid_t>(static_cast<uint32_t>(dst_task_tid)));
    if (dst_task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task::cid_t new_cid = task::move_capability(dst_task, task, cid_handle);
    if (new_cid.ccid == 0) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    return { .result = 0, .error = 0 };
  }

  sysret_t handle_system_call(uintptr_t function_id, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
    if (function_id >= array_size_of(handler_table)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    return handler_table[function_id](arg0, arg1, arg2, arg3, arg4, arg5);
  }
} // namespace caprese::syscall::cap
