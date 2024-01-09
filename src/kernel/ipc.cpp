#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <mutex>

#include <kernel/cls.h>
#include <kernel/ipc.h>
#include <kernel/log.h>
#include <kernel/task.h>
#include <kernel/user_memory.h>
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

  cur_task->ipc_short_msg[0] = arg0;
  cur_task->ipc_short_msg[1] = arg1;
  cur_task->ipc_short_msg[2] = arg2;
  cur_task->ipc_short_msg[3] = arg3;
  cur_task->ipc_short_msg[4] = arg4;
  cur_task->ipc_short_msg[5] = arg5;
  cur_task->ipc_msg_state    = ipc_msg_state_t::short_size;

  {
    std::lock_guard ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      {
        std::lock_guard lock(receiver->lock);

        ipc_transfer_ipc_msg(receiver, cur_task);

        remove_waiting_queue(endpoint->receiver_queue, receiver);
        receiver->state     = task_state_t::ready;
        receiver->ipc_state = ipc_state_t::none;
        receiver->endpoint  = 0_map;
      }

      switch_task(receiver);

      return true;
    }

    if (!blocking) {
      errno = SYS_E_BLOCKED;
      return false;
    }

    cur_task->state      = task_state_t::waiting;
    cur_task->ipc_state  = ipc_state_t::sending;
    cur_task->event_type = event_type_t::send;
    cur_task->endpoint   = endpoint;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);
  assert(cur_task->event_type == event_type_t::none);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_send_long(bool blocking, map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  assert(cur_task->prev_waiting_task == nullptr);
  assert(cur_task->next_waiting_task == nullptr);
  assert(cur_task->callee_task == nullptr);

  cur_task->ipc_long_msg  = msg;
  cur_task->ipc_msg_state = ipc_msg_state_t::long_size;

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      {
        std::lock_guard lock(receiver->lock);

        if (!ipc_transfer_ipc_msg(receiver, cur_task)) [[unlikely]] {
          return false;
        }

        remove_waiting_queue(endpoint->receiver_queue, receiver);
        receiver->state     = task_state_t::ready;
        receiver->ipc_state = ipc_state_t::none;
        receiver->endpoint  = 0_map;
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

    cur_task->state      = task_state_t::waiting;
    cur_task->ipc_state  = ipc_state_t::sending;
    cur_task->event_type = event_type_t::send;
    cur_task->endpoint   = endpoint;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);
  assert(cur_task->event_type == event_type_t::none);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_receive(bool blocking, map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  assert(cur_task->prev_waiting_task == nullptr);
  assert(cur_task->next_waiting_task == nullptr);
  assert(cur_task->callee_task == nullptr);

  cur_task->ipc_long_msg  = msg;
  cur_task->ipc_msg_state = ipc_msg_state_t::long_size;

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (cur_task->caller_task != nullptr) {
      map_ptr<task_t> caller = cur_task->caller_task;

      {
        std::lock_guard caller_lock(caller->lock);
        assert(caller->state == task_state_t::waiting);
        assert(caller->ipc_state == ipc_state_t::calling);
        assert(caller->event_type == event_type_t::send);
        assert(caller->prev_waiting_task == nullptr);
        assert(caller->next_waiting_task == nullptr);
        assert(caller->callee_task == cur_task);
        assert(caller->endpoint == endpoint);
        caller->state         = task_state_t::ready;
        caller->ipc_state     = ipc_state_t::canceled;
        caller->event_type    = event_type_t::none;
        caller->ipc_msg_state = ipc_msg_state_t::empty;
        caller->callee_task   = 0_map;
        caller->endpoint      = 0_map;
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

        if (sender->event_type == event_type_t::send) {
          if (!ipc_transfer_ipc_msg(cur_task, sender)) [[unlikely]] {
            return false;
          }

          if (sender->ipc_state == ipc_state_t::calling) {
            assert(sender->state == task_state_t::waiting);
            assert(sender->event_type == event_type_t::send);
            sender->callee_task   = cur_task;
            cur_task->caller_task = sender;
          } else {
            sender->state      = task_state_t::ready;
            sender->ipc_state  = ipc_state_t::none;
            sender->event_type = event_type_t::none;
            sender->endpoint   = 0_map;
          }
        } else if (sender->event_type == event_type_t::kill) {
          ipc_transfer_kill_msg(cur_task, sender);
        }

        remove_waiting_queue(endpoint->sender_queue, sender);
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
    cur_task->endpoint  = endpoint;
    push_waiting_queue(endpoint->receiver_queue, cur_task);
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);

  if (cur_task->ipc_state == ipc_state_t::canceled) {
    errno = SYS_E_CANCELED;
  }

  return cur_task->ipc_state == ipc_state_t::none;
}

bool ipc_reply(map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg) {
  std::unique_lock ep_lock(endpoint->lock);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  if (cur_task->caller_task == nullptr) {
    return true;
  }

  map_ptr<task_t> caller = cur_task->caller_task;

  assert(caller->callee_task == cur_task);
  assert(caller->state == task_state_t::waiting);
  assert(caller->ipc_state == ipc_state_t::calling);
  assert(caller->event_type == event_type_t::send);
  assert(caller->endpoint == endpoint);

  cur_task->ipc_long_msg  = msg;
  cur_task->ipc_msg_state = ipc_msg_state_t::long_size;

  {
    std::lock_guard caller_lock(caller->lock);

    if (!ipc_transfer_ipc_msg(caller, cur_task)) [[unlikely]] {
      return false;
    }

    caller->state         = task_state_t::ready;
    caller->ipc_state     = ipc_state_t::none;
    caller->event_type    = event_type_t::none;
    caller->callee_task   = 0_map;
    caller->endpoint      = 0_map;
    cur_task->caller_task = 0_map;

    push_ready_queue(caller);
  }

  return true;
}

bool ipc_call(map_ptr<endpoint_t> endpoint, virt_ptr<message_t> msg) {
  assert(endpoint != nullptr);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  cur_task->ipc_long_msg  = msg;
  cur_task->ipc_msg_state = ipc_msg_state_t::long_size;
  cur_task->state         = task_state_t::waiting;
  cur_task->ipc_state     = ipc_state_t::calling;
  cur_task->event_type    = event_type_t::send;
  cur_task->endpoint      = endpoint;

  {
    std::unique_lock ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      std::lock_guard recv_lock(receiver->lock);

      if (!ipc_transfer_ipc_msg(receiver, cur_task)) [[unlikely]] {
        return false;
      }

      remove_waiting_queue(endpoint->receiver_queue, receiver);

      receiver->state       = task_state_t::ready;
      receiver->ipc_state   = ipc_state_t::none;
      receiver->caller_task = cur_task;
      receiver->endpoint    = 0_map;
      cur_task->callee_task = receiver;

      push_ready_queue(receiver);
    } else {
      push_waiting_queue(endpoint->sender_queue, cur_task);
    }
  }

  resched();

  assert(cur_task->ipc_state == ipc_state_t::none || cur_task->ipc_state == ipc_state_t::canceled);
  assert(cur_task->event_type == event_type_t::none);

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
    receiver->endpoint  = 0_map;
    push_ready_queue(receiver);
  }

  while (endpoint->sender_queue.head != nullptr) {
    map_ptr<task_t> sender = endpoint->sender_queue.head;

    std::lock_guard lock(sender->lock);

    remove_waiting_queue(endpoint->sender_queue, sender);

    sender->ipc_msg_state = ipc_msg_state_t::empty;
    sender->state         = task_state_t::ready;
    sender->ipc_state     = ipc_state_t::canceled;
    sender->event_type    = event_type_t::none;
    sender->endpoint      = 0_map;
    push_ready_queue(sender);
  }
}

void ipc_send_kill_notify(map_ptr<endpoint_t> endpoint, map_ptr<task_t> task) {
  assert(endpoint != nullptr);
  assert(task != nullptr);
  assert(task->state == task_state_t::killed);

  {
    std::lock_guard ep_lock(endpoint->lock);

    if (endpoint->receiver_queue.head != nullptr) {
      assert(endpoint->sender_queue.head == nullptr);

      map_ptr<task_t> receiver = endpoint->receiver_queue.head;

      {
        std::lock_guard lock(receiver->lock);

        ipc_transfer_kill_msg(receiver, task);

        remove_waiting_queue(endpoint->receiver_queue, receiver);
        receiver->state     = task_state_t::ready;
        receiver->ipc_state = ipc_state_t::none;
        receiver->endpoint  = 0_map;
        push_ready_queue(receiver);
      }
    } else {
      task->ipc_state  = ipc_state_t::sending;
      task->event_type = event_type_t::kill;
      task->endpoint   = endpoint;
      push_waiting_queue(endpoint->sender_queue, task);
    }
  }

  resched();
}

bool ipc_transfer_ipc_msg(map_ptr<task_t> dst, map_ptr<task_t> src) {
  assert(dst != nullptr);
  assert(src != nullptr);

  if (dst->ipc_msg_state != ipc_msg_state_t::long_size) [[unlikely]] {
    panic("Unexpected ipc msg state: %d", dst->ipc_msg_state);
  }

  message_header src_msg_header;

  switch (src->ipc_msg_state) {
    case ipc_msg_state_t::empty:
      src_msg_header.payload_length   = 0;
      src_msg_header.data_type_map[0] = 0;
      src_msg_header.data_type_map[1] = 0;
      break;
    case ipc_msg_state_t::short_size:
      src_msg_header.payload_length   = sizeof(src->ipc_short_msg);
      src_msg_header.data_type_map[0] = 0;
      src_msg_header.data_type_map[1] = 0;
      break;
    case ipc_msg_state_t::long_size:
      if (!read_user_memory(src, src->ipc_long_msg.raw(), make_map_ptr(&src_msg_header), sizeof(message_header))) [[unlikely]] {
        loge(tag, "Failed to read user memory: tid=%d, addr=%p", src->tid, src->ipc_long_msg);
        return false;
      }
      break;
    default:
      panic("Unknown ipc msg state: %d", src->ipc_msg_state);
  }

  message_header dst_msg_header;
  if (!read_user_memory(dst, dst->ipc_long_msg.raw(), make_map_ptr(&dst_msg_header), sizeof(message_header))) [[unlikely]] {
    loge(tag, "Failed to read user memory: tid=%d, addr=%p", src->tid, src->ipc_long_msg);
    return false;
  }

  if (dst_msg_header.payload_capacity < src_msg_header.payload_length) [[unlikely]] {
    loge(tag, "Payload capacity is too small: tid=%d, capacity=%d, length=%d", dst->tid, dst_msg_header.payload_capacity, src_msg_header.payload_length);
    return false;
  }

  dst_msg_header.msg_type       = MSG_TYPE_IPC;
  dst_msg_header.sender_id      = std::bit_cast<uint32_t>(src->tid);
  dst_msg_header.receiver_id    = std::bit_cast<uint32_t>(dst->tid);
  dst_msg_header.payload_length = src_msg_header.payload_length;

  memcpy(&dst_msg_header.data_type_map, &src_msg_header.data_type_map, sizeof(src_msg_header.data_type_map));

  if (!write_user_memory(dst, make_map_ptr(&dst_msg_header), dst->ipc_long_msg.raw(), sizeof(message_header))) [[unlikely]] {
    loge(tag, "Failed to write user memory: tid=%d, addr=%p", dst->tid, dst->ipc_long_msg);
    return false;
  }

  constexpr size_t xlen = 8 * sizeof(dst_msg_header.data_type_map[0]);

  switch (src->ipc_msg_state) {
    case ipc_msg_state_t::empty:
      // Do nothing.
      break;
    case ipc_msg_state_t::short_size:
      if (!write_user_memory(dst, make_map_ptr(src->ipc_short_msg), dst->ipc_long_msg.raw() + sizeof(message_header), sizeof(src->ipc_short_msg))) [[unlikely]] {
        loge(tag, "Failed to write user memory: tid=%d, addr=%p", dst->tid, dst->ipc_long_msg);
        return false;
      }
      break;
    case ipc_msg_state_t::long_size:
      if (!forward_user_memory(src, dst, src->ipc_long_msg.raw() + sizeof(message_header), dst->ipc_long_msg.raw() + sizeof(message_header), dst_msg_header.payload_length)) [[unlikely]] {
        loge(tag, "Failed to forward user memory: tid=%d, src_addr=%p, dst_addr=%p, length=%d", src->tid, src->ipc_long_msg, dst->ipc_long_msg, dst_msg_header.payload_length);
        return false;
      }

      for (size_t i = 0; i < std::min(dst_msg_header.payload_length / sizeof(uintptr_t), xlen * std::size(dst_msg_header.data_type_map)); ++i) {
        if (dst_msg_header.data_type_map[i / xlen] & (1ull << (i % xlen))) [[unlikely]] {
          uintptr_t value;
          if (!read_user_memory(src, src->ipc_long_msg.raw() + sizeof(message_header) + sizeof(uintptr_t) * i, make_map_ptr(&value), sizeof(uintptr_t))) [[unlikely]] {
            loge(tag, "Failed to read user memory: tid=%d, addr=%p", src->tid, src->ipc_long_msg.raw() + sizeof(message_header) + sizeof(uintptr_t) * i);
            return false;
          }

          uintptr_t cap_desc = (value << 1) >> 1;
          bool      delegate = cap_desc != value;

          map_ptr<cap_slot_t> dst_slot;
          if (delegate) {
            dst_slot = delegate_cap(dst, lookup_cap(src, cap_desc));
          } else {
            dst_slot = transfer_cap(dst, lookup_cap(src, cap_desc));
          }

          if (dst_slot == nullptr) [[unlikely]] {
            loge(tag, "Failed to %s cap: tid=%d, cap_desc=%p", delegate ? "delegate" : "transfer", src->tid, cap_desc);
            return false;
          }

          uintptr_t dst_cap_desc = get_cap_slot_index(dst_slot);
          if (!write_user_memory(dst, make_map_ptr(&dst_cap_desc), dst->ipc_long_msg.raw() + sizeof(message_header) + sizeof(uintptr_t) * i, sizeof(uintptr_t))) [[unlikely]] {
            loge(tag, "Failed to write user memory: tid=%d, addr=%p", dst->tid, dst->ipc_long_msg.raw() + sizeof(message_header) + sizeof(uintptr_t) * i);
            return false;
          }
        }
      }
      break;
    default:
      panic("Unknown ipc msg state: %d", src->ipc_msg_state);
  }

  src->ipc_msg_state = ipc_msg_state_t::empty;
  dst->ipc_msg_state = ipc_msg_state_t::empty;

  return true;
}

bool ipc_transfer_kill_msg(map_ptr<task_t> dst, map_ptr<task_t> src) {
  assert(dst != nullptr);
  assert(src != nullptr);

  if (dst->ipc_msg_state != ipc_msg_state_t::long_size) [[unlikely]] {
    panic("Unexpected ipc msg state: %d", dst->ipc_msg_state);
  }

  message_header header;
  if (!read_user_memory(dst, dst->ipc_long_msg.raw(), make_map_ptr(&header), sizeof(message_header))) [[unlikely]] {
    loge(tag, "Failed to read user memory: tid=%d, addr=%p", dst->tid, dst->ipc_long_msg);
    return false;
  }

  header.msg_type         = MSG_TYPE_KILL;
  header.sender_id        = std::bit_cast<uint32_t>(src->tid);
  header.receiver_id      = std::bit_cast<uint32_t>(dst->tid);
  header.payload_length   = 0;
  header.data_type_map[0] = 0;
  header.data_type_map[1] = 0;

  if (!write_user_memory(dst, make_map_ptr(&header), dst->ipc_long_msg.raw(), sizeof(message_header))) [[unlikely]] {
    loge(tag, "Failed to write user memory: tid=%d, addr=%p", dst->tid, dst->ipc_long_msg);
    return false;
  }

  dst->ipc_msg_state = ipc_msg_state_t::empty;

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
