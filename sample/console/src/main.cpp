#include <cstdio>

#include <console/ipc.h>
#include <runtime/global.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>
#include <runtime/task.h>

extern "C" int main() {
  printf("Hello, console!\n");

  ipc_msg_t msg;

  while (true) {
    if (!ipc_receive(&msg)) {
      continue;
    }

    if (msg.channel == CONSOLE_IPC_CHANNEL_READ) {
      if ((msg.flags & IPC_MSG_FLAG_TYPE_MASK) != IPC_MSG_FLAG_TYPE_MSG) [[unlikely]] {
        printf("[WARN] Invalid message type: %d\n", msg.flags & IPC_MSG_FLAG_TYPE_MASK);
      }
      printf("Input:\n");
      sysret_t sysret = sys_debug_getchar();
      if (sysret.error) {
        printf("[ERROR]\n");
      } else {
        printf("get char: %c\n", static_cast<char>(sysret.result));
      }
    } else if (msg.channel == CONSOLE_IPC_CHANNEL_WRITE) {
      if ((msg.flags & IPC_MSG_FLAG_TYPE_MASK) != IPC_MSG_FLAG_TYPE_MSG) [[unlikely]] {
        printf("[WARN] Invalid message type: %d\n", msg.flags & IPC_MSG_FLAG_TYPE_MASK);
      }
      const char* va = static_cast<char*>(map_memory_page_to_heap(msg.msg.handles[0], MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE));
      if (va == nullptr) [[unlikely]] {
        printf("[INFO] Failed to map memory page.\n");
        continue;
      }
      printf("[ECHO]\n");
      for (size_t i = 0; i < __page_size; ++i) {
        if (va[i] == '\0') [[unlikely]] {
          break;
        }
        sys_debug_putchar(va[i]);
      }
    } else {
      printf("Unknown channel: %u\n", msg.channel);
    }
  }

  return 0;
}
