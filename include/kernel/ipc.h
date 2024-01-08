#ifndef KERNEL_IPC_H_
#define KERNEL_IPC_H_

#include <kernel/address.h>
#include <kernel/cap.h>
#include <kernel/lock.h>

struct task_t;

struct task_queue_t {
  map_ptr<task_t> head;
  map_ptr<task_t> tail;
};

struct endpoint_t {
  task_queue_t         sender_queue;
  task_queue_t         receiver_queue;
  recursive_spinlock_t lock;
};

static_assert(sizeof(endpoint_t) <= get_cap_size(CAP_ENDPOINT));

bool ipc_send_short(bool blocking, map_ptr<endpoint_t> endpoint, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
bool ipc_send_long(bool blocking, map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg);
bool ipc_receive(bool blocking, map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg);
bool ipc_reply(map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg);
bool ipc_call(map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg);
void ipc_cancel(map_ptr<endpoint_t> endpoint);
void ipc_send_kill_notify(map_ptr<endpoint_t> endpoint, map_ptr<task_t> task);

bool ipc_transfer_ipc_msg(map_ptr<task_t> dst, map_ptr<task_t> src);
bool ipc_transfer_kill_msg(map_ptr<task_t> dst, map_ptr<task_t> src);

void push_waiting_queue(task_queue_t& queue, map_ptr<task_t> task);
void remove_waiting_queue(task_queue_t& queue, map_ptr<task_t> task);

#endif // KERNEL_IPC_H_
