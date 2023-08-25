#include <algorithm>
#include <cstdio>
#include <cstring>

#include <caprese/capability/bic/task.h>

#include <apm/ipc.h>
#include <runtime/global.h>
#include <runtime/syscall.h>
#include <runtime/task.h>

extern "C" {
  tid_t get_tid(void) {
    sysret_t sysret = sys_task_tid();
    if (sysret.error) [[unlikely]] {
      return 0;
    }
    return static_cast<tid_t>(sysret.result);
  }

  task_handle_t create_task() {
    sysret_t sysret = sys_task_create();
    if (sysret.error) [[unlikely]] {
      return 0;
    }
    return static_cast<task_handle_t>(sysret.result);
  }

  void yield() {
    sys_task_yield();
  }

  task_state_t task_state(task_handle_t task) {
    sysret_t sysret = sys_cap_call_method(task, caprese::capability::bic::task::method::STATE, 0, 0, 0, 0);
    if (sysret.error) [[unlikely]] {
      return TASK_STATE_UNUSED;
    }
    return sysret.result;
  }

  bool kill_task(task_handle_t task, uintptr_t exit_code) {
    sysret_t sysret = sys_cap_call_method(task, caprese::capability::bic::task::method::KILL, exit_code, 0, 0, 0);
    return sysret.error == 0;
  }

  void switch_task(task_handle_t task) {
    if (task == __this_task_handle) [[unlikely]] {
      return;
    }
    sys_cap_call_method(task, caprese::capability::bic::task::method::SWITCH, 0, 0, 0, 0);
  }

  bool set_register(task_handle_t task, uintptr_t reg, uintptr_t value) {
    sysret_t sysret = sys_cap_call_method(task, caprese::capability::bic::task::method::SET_REGISTER, reg, value, 0, 0);
    return sysret.error == 0;
  }

  bool ipc_send(task_handle_t task, const ipc_msg_t* msg) {
    sysret_t sysret = sys_cap_call_method(task, caprese::capability::bic::task::method::SEND, reinterpret_cast<uintptr_t>(msg), 0, 0, 0);
    if (sysret.error) [[unlikely]] {
      return false;
    }
    return (msg->flags & IPC_MSG_FLAG_CANCELED) == 0;
  }

  bool ipc_receive(ipc_msg_t* msg) {
    sysret_t sysret = sys_task_ipc_receive(msg);
    return sysret.error == 0;
  }

  task_handle_t this_task_handle(void) {
    return __this_task_handle;
  }

  task_handle_t lookup_task(const char* name) {
    if (__apm_task_handle == 0) [[unlikely]] {
      return 0;
    }

    ipc_msg_t msg {};
    msg.flags   = IPC_MSG_FLAG_TYPE_SHORT_MSG;
    msg.channel = APM_IPC_CHANNEL_LOOKUP;
    strncpy(reinterpret_cast<char*>(msg.short_msg.data), name, std::min(16uz, strlen(name)));
    ipc_send(__apm_task_handle, &msg);

    return 0;
  }
}
