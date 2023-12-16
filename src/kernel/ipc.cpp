#include <cerrno>
#include <mutex>

#include <kernel/cls.h>
#include <kernel/ipc.h>
#include <kernel/log.h>
#include <kernel/task.h>
#include <libcaprese/syscall.h>

namespace {
  constexpr const char* tag = "kernel/ipc";
} // namespace

bool ipc_send_short(bool blocking, map_ptr<endpoint_t> endpoint, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  assert(cur_task->prev_waiting_task == nullptr);
  assert(cur_task->next_waiting_task == nullptr);
  assert(cur_task->callee_task == nullptr);

  {
    std::lock_guard ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      {
        std::lock_guard lock(receiver->lock);

        receiver->msg_buf.cap_part_length  = 0;
        receiver->msg_buf.data_part_length = 6;
        receiver->msg_buf.data[0]          = arg0;
        receiver->msg_buf.data[1]          = arg1;
        receiver->msg_buf.data[2]          = arg2;
        receiver->msg_buf.data[3]          = arg3;
        receiver->msg_buf.data[4]          = arg4;
        receiver->msg_buf.data[5]          = arg5;

        remove_waiting_queue(endpoint->receiver_queue, receiver);
        receiver->state     = task_state_t::ready;
        receiver->ipc_state = ipc_state_t::none;
      }

      switch_task(receiver);

      return true;
    }

    if (!blocking) {
      errno = SYS_E_BLOCKED;
      return false;
    }

    cur_task->msg_buf.cap_part_length  = 0;
    cur_task->msg_buf.data_part_length = 6;
    cur_task->msg_buf.data[0]          = arg0;
    cur_task->msg_buf.data[1]          = arg1;
    cur_task->msg_buf.data[2]          = arg2;
    cur_task->msg_buf.data[3]          = arg3;
    cur_task->msg_buf.data[4]          = arg4;
    cur_task->msg_buf.data[5]          = arg5;

    cur_task->state     = task_state_t::waiting;
    cur_task->ipc_state = ipc_state_t::sending;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  // The message should be written to receiver->msg_buf in ipc_receive.
  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_send_long(bool blocking, map_ptr<endpoint_t> endpoint) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  assert(cur_task->prev_waiting_task == nullptr);
  assert(cur_task->next_waiting_task == nullptr);
  assert(cur_task->callee_task == nullptr);

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      {
        std::lock_guard lock(receiver->lock);

        if (!ipc_transfer_msg(receiver, cur_task)) [[unlikely]] {
          return false;
        }

        remove_waiting_queue(endpoint->receiver_queue, receiver);
        receiver->state     = task_state_t::ready;
        receiver->ipc_state = ipc_state_t::none;
      }

      ep_lock.unlock();
      switch_task(receiver);
      ep_lock.lock();

      return true;
    }

    if (!blocking) {
      errno = SYS_E_BLOCKED;
      return false;
    }

    cur_task->state     = task_state_t::waiting;
    cur_task->ipc_state = ipc_state_t::sending;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  // The message should be written to receiver->msg_buf in ipc_receive.
  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_receive(bool blocking, map_ptr<endpoint_t> endpoint) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  assert(cur_task->prev_waiting_task == nullptr);
  assert(cur_task->next_waiting_task == nullptr);
  assert(cur_task->callee_task == nullptr);

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (cur_task->caller_task != nullptr) {
      map_ptr<task_t> caller = cur_task->caller_task;

      {
        std::lock_guard caller_lock(caller->lock);
        assert(caller->state == task_state_t::waiting);
        assert(caller->ipc_state == ipc_state_t::calling);
        assert(caller->prev_waiting_task == nullptr);
        assert(caller->next_waiting_task == nullptr);
        assert(caller->callee_task == cur_task);
        caller->state       = task_state_t::ready;
        caller->ipc_state   = ipc_state_t::canceled;
        caller->callee_task = 0_map;
        push_ready_queue(caller);
      }

      cur_task->caller_task = 0_map;
    }

    if (endpoint->sender_queue.head != nullptr) {
      assert(endpoint->receiver_queue.head == nullptr);

      map_ptr<task_t> sender = endpoint->sender_queue.head;

      {
        std::lock_guard lock(sender->lock);

        assert(sender->callee_task == nullptr);

        if (!ipc_transfer_msg(cur_task, sender)) [[unlikely]] {
          return false;
        }

        remove_waiting_queue(endpoint->sender_queue, sender);

        if (sender->ipc_state == ipc_state_t::calling) {
          assert(sender->state == task_state_t::waiting);
          sender->callee_task   = cur_task;
          cur_task->caller_task = sender;
        } else {
          sender->state     = task_state_t::ready;
          sender->ipc_state = ipc_state_t::none;
        }
      }

      if (sender->state == task_state_t::ready) {
        ep_lock.unlock();
        switch_task(sender);
        ep_lock.lock();
      }

      return true;
    }

    if (!blocking) {
      errno = SYS_E_BLOCKED;
      return false;
    }

    cur_task->state     = task_state_t::waiting;
    cur_task->ipc_state = ipc_state_t::receiving;
    push_waiting_queue(endpoint->receiver_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  // The message should be written to cur_task->msg_buf in ipc_send_xxx.
  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_reply(map_ptr<endpoint_t> endpoint) {
  std::unique_lock ep_lock(endpoint->lock);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  if (cur_task->caller_task == nullptr) {
    return true;
  }

  map_ptr<task_t> caller = cur_task->caller_task;

  assert(caller->callee_task == cur_task);
  assert(caller->state == task_state_t::waiting);
  assert(caller->ipc_state == ipc_state_t::calling);

  {
    std::lock_guard caller_lock(caller->lock);

    if (!ipc_transfer_msg(caller, cur_task)) [[unlikely]] {
      return false;
    }

    caller->state         = task_state_t::ready;
    caller->ipc_state     = ipc_state_t::none;
    caller->callee_task   = 0_map;
    cur_task->caller_task = 0_map;

    push_ready_queue(caller);
  }

  return true;
}

bool ipc_call(map_ptr<endpoint_t> endpoint) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  cur_task->state     = task_state_t::waiting;
  cur_task->ipc_state = ipc_state_t::calling;

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      std::lock_guard recv_lock(receiver->lock);

      if (!ipc_transfer_msg(receiver, cur_task)) [[unlikely]] {
        return false;
      }

      remove_waiting_queue(endpoint->receiver_queue, receiver);

      receiver->state       = task_state_t::ready;
      receiver->ipc_state   = ipc_state_t::none;
      receiver->caller_task = cur_task;
      cur_task->callee_task = receiver;

      push_ready_queue(receiver);
    } else {
      push_waiting_queue(endpoint->sender_queue, cur_task);
    }
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  return cur_task->ipc_state == ipc_state_t::none;
}

void ipc_cancel(map_ptr<endpoint_t> endpoint) {
  std::unique_lock ep_lock(endpoint->lock);

  while (endpoint->receiver_queue.head != nullptr) {
    map_ptr<task_t> receiver = endpoint->receiver_queue.head;

    std::lock_guard lock(receiver->lock);

    remove_waiting_queue(endpoint->receiver_queue, receiver);

    receiver->state     = task_state_t::ready;
    receiver->ipc_state = ipc_state_t::canceled;
    push_ready_queue(receiver);
  }

  while (endpoint->sender_queue.head != nullptr) {
    map_ptr<task_t> sender = endpoint->sender_queue.head;

    std::lock_guard lock(sender->lock);

    remove_waiting_queue(endpoint->sender_queue, sender);

    sender->state     = task_state_t::ready;
    sender->ipc_state = ipc_state_t::canceled;
    push_ready_queue(sender);
  }
}

bool ipc_transfer_msg(map_ptr<task_t> dst, map_ptr<task_t> src) {
  assert(dst != nullptr);
  assert(src != nullptr);

  const message_buffer_t& src_msg_buf = src->msg_buf;
  message_buffer_t&       dst_msg_buf = dst->msg_buf;

  for (size_t i = 0; i < src_msg_buf.cap_part_length; ++i) {
    uintptr_t cap_desc = (src_msg_buf.data[i] << 1) >> 1;
    bool      delegate = cap_desc != src_msg_buf.data[i];

    map_ptr<cap_slot_t> src_slot = lookup_cap(src, cap_desc);
    if (src_slot == nullptr) [[unlikely]] {
      logd(tag, "Failed to transfer the message. Failed to lookup cap from the source buffer. (index=%llu, cap=%llu)", i, src_msg_buf.data[i]);
      return false;
    }

    map_ptr<cap_slot_t> dst_slot = delegate ? delegate_cap(dst, src_slot) : transfer_cap(dst, src_slot);
    if (dst_slot == nullptr) [[unlikely]] {
      logd(tag, "Failed to transfer the message. Failed to %s cap to the destination buffer.", delegate ? "delegate" : "transfer");
      return false;
    }

    dst_msg_buf.data[i] = get_cap_slot_index(dst_slot);
  }

  for (size_t i = 0; i < src_msg_buf.data_part_length; ++i) {
    dst_msg_buf.data[src_msg_buf.cap_part_length + i] = src_msg_buf.data[src_msg_buf.cap_part_length + i];
  }

  dst_msg_buf.cap_part_length  = src_msg_buf.cap_part_length;
  dst_msg_buf.data_part_length = src_msg_buf.data_part_length;

  return true;
}

void push_waiting_queue(task_queue_t& queue, map_ptr<task_t> task) {
  assert(task != nullptr);
  assert(task->state == task_state_t::waiting);
  assert(task->prev_waiting_task == nullptr);
  assert(task->next_waiting_task == nullptr);

  if (queue.head == nullptr) {
    queue.head = task;
    queue.tail = task;
  } else {
    queue.tail->next_waiting_task = task;
    task->prev_waiting_task       = queue.tail;
    queue.tail                    = task;
  }
}

void remove_waiting_queue(task_queue_t& queue, map_ptr<task_t> task) {
  assert(task != nullptr);
  assert(task->state == task_state_t::waiting);

  if (queue.head == task) {
    queue.head = task->next_waiting_task;
  } else {
    task->prev_waiting_task->next_waiting_task = task->next_waiting_task;
  }

  if (queue.tail == task) {
    queue.tail = task->prev_waiting_task;
  } else {
    task->next_waiting_task->prev_waiting_task = task->prev_waiting_task;
  }

  task->prev_waiting_task = 0_map;
  task->next_waiting_task = 0_map;
}
