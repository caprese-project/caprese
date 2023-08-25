#include <cstdio>

#include <console/ipc.h>
#include <runtime/global.h>
#include <runtime/task.h>

extern "C" int main() {
  printf("Hello, shell!\n");

  while (true) {
    ipc_msg_t msg;
    msg.flags   = IPC_MSG_FLAG_TYPE_SHORT_MSG;
    msg.channel = CONSOLE_IPC_CHANNEL_READ;
    ipc_send(__console_task_handle, &msg);
  }

  return 0;
}
