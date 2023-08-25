#ifndef CAPRESE_TASK_IPC_H_
#define CAPRESE_TASK_IPC_H_

#include <caprese/memory/address.h>
#include <caprese/task/types.h>

namespace caprese::task {
  [[nodiscard]] bool is_valid_msg(const msg_t* msg);

  [[nodiscard]] bool ipc_send(task_t* receiver, memory::user_address_t msg);
  [[nodiscard]] bool ipc_nb_send(task_t* receiver, memory::user_address_t msg);
  [[nodiscard]] bool ipc_receive(memory::user_address_t msg);

  void ipc_cancel(task_t* task);
} // namespace caprese::task

#endif // CAPRESE_TASK_IPC_H_
