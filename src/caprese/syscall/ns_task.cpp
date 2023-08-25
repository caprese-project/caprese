#include <bit>

#include <caprese/capability/bic/task.h>
#include <caprese/syscall/ns_task.h>
#include <caprese/task/cap.h>
#include <caprese/task/ipc.h>
#include <caprese/task/sched.h>
#include <caprese/task/task.h>
#include <caprese/util/array.h>

namespace caprese::syscall::task {
  using namespace caprese::task;

  namespace {
    using handler_t = sysret_t (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

    constexpr handler_t handler_table[] = {
      [TID_FID]     = sys_tid,
      [CREATE_FID]  = sys_create,
      [YIELD_FID]   = sys_yield,
      [IPC_RECEIVE] = sys_ipc_receive,
    };
  } // namespace

  sysret_t sys_tid([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    return { .result = std::bit_cast<uint32_t>(get_current_task()->tid), .error = 0 };
  }

  sysret_t sys_create([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    task_t* task = create_task();
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability::capability_t* cap = capability::bic::task::create(task->tid);
    if (cap == nullptr) [[unlikely]] {
      kill(task);
      return { .result = 0, .error = 1 };
    }

    cid_handle_t handle = insert_capability(get_current_task(), cap);
    if (!handle) [[unlikely]] {
      capability::delete_capability(cap);
      kill(task);
      return { .result = 0, .error = 1 };
    }

    return { .result = handle, .error = 0 };
  }

  sysret_t sys_yield([[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    yield();
    return { .result = 0, .error = 0 };
  }

  sysret_t sys_ipc_receive(uintptr_t msg_addr, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t, [[maybe_unused]] uintptr_t) {
    bool result = ipc_receive(memory::user_address_t::from(msg_addr));
    if (!result) [[unlikely]] {
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
} // namespace caprese::syscall::task
