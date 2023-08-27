#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include <caprese/capability/bic/task.h>

#include <apm/ipc.h>
#include <runtime/cap.h>
#include <runtime/elf.h>
#include <runtime/global.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>
#include <runtime/task.h>
#include <runtime/types.h>

namespace {
  std::unordered_map<std::string, const char*>   app_table;
  std::unordered_map<std::string, task_handle_t> lookup_table;

  task_handle_t create(const void* elf) {
    task_handle_t task_handle = create_task();
    if (task_handle == 0) [[unlikely]] {
      return 0;
    }
    if (!load_elf(task_handle, elf)) [[unlikely]] {
      return 0;
    }

    sysret_t sysret = sys_cap_list_size();
    if (sysret.error) [[unlikely]] {
      printf("Failed to load cap list size.\n");
      abort();
    }

    size_t count = 0;
    for (handle_t handle = 0; handle < sysret.result; ++handle) {
      if (get_handle_type(handle) != HANDLE_TYPE_MEMORY) {
        continue;
      }

      if (!is_mapped(handle)) {
        if (sys_cap_move(handle, task_handle).error) [[unlikely]] {
          printf("Failed to move memory cap.\n");
          abort();
        }
        if (++count > 100) {
          break;
        }
      }
    }

    return task_handle;
  }

  bool exec(const std::string& app_name) {
    if (lookup_table.contains(app_name)) [[unlikely]] {
      return false;
    }
    if (!app_table.contains(app_name)) [[unlikely]] {
      return false;
    }

    task_handle_t task_handle = create(app_table.at(app_name));
    if (task_handle == 0) [[unlikely]] {
      return false;
    }
    lookup_table[app_name] = task_handle;

    printf("Switch to %s\n", app_name.c_str());
    switch_task(task_handle);

    return true;
  }
} // namespace

extern "C" {
  extern const char _console_start[];
  extern const char _shell_start[];
  extern const char _echo_start[];
  extern const char _pingpong_start[];

  int main() {
    __apm_task_handle = __this_task_handle;

    printf("Hello, apm!\n");

    task_handle_t ping = create(_pingpong_start);
    task_handle_t pong = create(_pingpong_start);
    assert(ping);
    assert(pong);

    task_handle_t ping_copy = sys_cap_copy(ping, caprese::capability::bic::task::permission::ALL).result;
    task_handle_t pong_copy = sys_cap_copy(pong, caprese::capability::bic::task::permission::ALL).result;
    assert(ping_copy);
    assert(pong_copy);

    task_handle_t pong_moved = sys_cap_move(pong_copy, ping).result;
    assert(set_register(ping, caprese::arch::REGISTER_S0, pong_moved));
    assert(pong_moved);

    task_handle_t ping_moved = sys_cap_move(ping_copy, pong).result;
    assert(set_register(pong, caprese::arch::REGISTER_S0, ping_moved));
    assert(ping_moved);

    switch_task(ping);

    app_table["console"] = _console_start;
    app_table["shell"]   = _shell_start;
    app_table["echo"]    = _echo_start;

    if (!exec("console")) [[unlikely]] {
      printf("Failed to exec console task.\n");
      abort();
    }
    if (!exec("shell")) [[unlikely]] {
      printf("Failed to exec shell task.\n");
      abort();
    }

    ipc_msg_t msg;
    while (true) {
      if (!ipc_receive(&msg)) {
        continue;
      }

      if (msg.channel == APM_IPC_CHANNEL_LOOKUP) {
        if ((msg.flags & IPC_MSG_FLAG_TYPE_MASK) != IPC_MSG_FLAG_TYPE_SHORT_MSG) [[unlikely]] {
          printf("[WARN] Invalid ipc msg type: %u\n", msg.flags & IPC_MSG_FLAG_TYPE_MASK);
          continue;
        }
        std::string name(reinterpret_cast<char*>(msg.short_msg.data), 16);
        if (!lookup_table.contains(name)) [[unlikely]] {
          printf("[WARN] Unknown app name: %s\n", name.c_str());
          continue;
        }
        printf("[INFO] app name: %s\n", name.c_str());
      } else {
        printf("[WARN] Invalid ipc channel: %u\n", msg.channel);
      }
    }

    return 0;
  }
}
