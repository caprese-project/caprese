#include <bit>
#include <cassert>
#include <cstdio>

#include <caprese/capability/bic/memory.h>
#include <caprese/memory/page.h>
#include <caprese/task/cap.h>
#include <caprese/task/ipc.h>
#include <caprese/task/sched.h>
#include <caprese/task/task.h>
#include <caprese/util/array.h>

namespace caprese::task {
  namespace {
    constexpr size_t handles_per_page = arch::PAGE_SIZE / sizeof(cid_handle_t);

    [[nodiscard]] size_t long_msg_level(size_t msg_size) {
      size_t num_pages = (msg_size + arch::PAGE_SIZE - 1) / arch::PAGE_SIZE;

      size_t level = 1;
      size_t size  = handles_per_page;
      while (size < num_pages) {
        size_t next_size = size * handles_per_page;
        if (next_size < size) [[unlikely]] {
          return 0;
        }
        size = next_size;
        ++level;
      }

      return level;
    }

    [[nodiscard]] cid_handle_t move_long_msg(task_t* dst_task, task_t* src_task, cid_handle_t table_handle, size_t level) {
      if (level == 0) {
        return move_capability(dst_task, src_task, table_handle);
      }

      cid_t* cid = lookup_cid(src_task, table_handle);
      if (cid == nullptr) [[unlikely]] {
        return 0;
      }
      capability::capability_t* cap = lookup_capability(src_task, *cid);
      if (cap == nullptr) [[unlikely]] {
        return 0;
      }
      if (cap->ccid != capability::bic::memory::CCID) [[unlikely]] {
        return 0;
      }

      auto [physical_address, physical_address_error] = capability::get_field(cap, capability::bic::memory::field::PHYSICAL_ADDRESS);
      if (physical_address_error) [[unlikely]] {
        return 0;
      }

      memory::mapped_address_t page  = memory::physical_address_t::from(physical_address).mapped_address();
      cid_handle_t*            table = page.as<cid_handle_t>();
      for (size_t i = 0; i < handles_per_page; ++i) {
        if (table[i] == 0) [[unlikely]] {
          break;
        }
        cid_handle_t handle = move_long_msg(dst_task, src_task, table[i], level - 1);
        table[i]            = handle;
      }

      return move_capability(dst_task, src_task, table_handle);
    }

    void move_msg(msg_t* dst, task_t* src_task, const msg_t* msg) {
      task_t* task = get_current_task();

      uint32_t type = msg->flags & MSG_FLAG_TYPE_MASK;

      if (type == MSG_FLAG_TYPE_SHORT_MSG) {
        dst->flags   = MSG_FLAG_TYPE_SHORT_MSG;
        dst->channel = msg->channel;

        for (size_t i = 0; i < array_size_of(msg->short_msg.data); ++i) {
          dst->short_msg.data[i] = msg->short_msg.data[i];
        }
      } else if (type == MSG_FLAG_TYPE_MSG) {
        dst->flags   = MSG_FLAG_TYPE_MSG;
        dst->channel = msg->channel;

        for (size_t i = 0; i < array_size_of(msg->msg.handles); ++i) {
          dst->msg.handles[i] = move_capability(task, src_task, msg->msg.handles[i]);
        }
      } else if (type == MSG_FLAG_TYPE_LONG_MSG) {
        dst->flags   = MSG_FLAG_TYPE_LONG_MSG;
        dst->channel = msg->channel;

        size_t level               = long_msg_level(msg->long_msg.msg_size);
        dst->long_msg.msg_size     = msg->long_msg.msg_size;
        dst->long_msg.table_handle = move_long_msg(task, src_task, msg->long_msg.table_handle, level);
      }
    }

    [[nodiscard]] bool is_valid_long_msg(task_t* task, cid_handle_t table_handle, size_t level) {
      if (level == 0) {
        if (table_handle == 0) [[unlikely]] {
          return true;
        }
        cid_t* cid = lookup_cid(task, table_handle);
        if (cid == nullptr) [[unlikely]] {
          return false;
        }
        capability::capability_t* cap = lookup_capability(task, *cid);
        if (cap == nullptr) [[unlikely]] {
          return false;
        }
        return cap->ccid == capability::bic::memory::CCID;
      }

      cid_t* cid = lookup_cid(task, table_handle);
      if (cid == nullptr) [[unlikely]] {
        return false;
      }
      capability::capability_t* cap = lookup_capability(task, *cid);
      if (cap == nullptr) [[unlikely]] {
        return false;
      }
      if (cap->ccid != capability::bic::memory::CCID) [[unlikely]] {
        return false;
      }

      auto [physical_address, physical_address_error] = capability::get_field(cap, capability::bic::memory::field::PHYSICAL_ADDRESS);
      if (physical_address_error) [[unlikely]] {
        return false;
      }

      memory::mapped_address_t page  = memory::physical_address_t::from(physical_address).mapped_address();
      cid_handle_t*            table = page.as<cid_handle_t>();
      for (size_t i = 0; i < handles_per_page; ++i) {
        if (table[i] == 0) [[unlikely]] {
          break;
        }
        if (!is_valid_long_msg(task, table[i], level - 1)) [[unlikely]] {
          return false;
        }
      }

      return true;
    }

    [[nodiscard]] bool ipc_fastpath(task_t* receiver, task_t* sender, const msg_t* msg) {
      assert(receiver->state == TASK_STATE_WAITING && receiver->prev_waiting_queue == null_tid && receiver->next_sender_task == null_tid);
      move_msg(receiver->msg.as<msg_t>(), sender, msg);
      receiver->msg.as<msg_t>()->flags |= MSG_FLAG_FASTPATH;
      receiver->next_sender_task = sender->tid;
      wakeup(receiver);
      switch_to(receiver);
      return true;
    }
  } // namespace

  bool is_valid_msg(task_t* sender, const msg_t* msg) {
    if ((msg->flags & MSG_FLAG_TYPE_MASK) == 0) [[unlikely]] {
      return false;
    }

    uint32_t type = msg->flags & MSG_FLAG_TYPE_MASK;

    if (type == MSG_FLAG_TYPE_SHORT_MSG) {
      return true;
    } else if (type == MSG_FLAG_TYPE_MSG) {
      for (cid_handle_t cid_handle : msg->msg.handles) {
        if (cid_handle == 0) {
          continue;
        }
        cid_t* cid = lookup_cid(sender, cid_handle);
        if (cid == nullptr) [[unlikely]] {
          return false;
        }
        if (cid->ccid != capability::bic::memory::CCID) [[unlikely]] {
          return false;
        }
      }

      return true;
    } else if (type == MSG_FLAG_TYPE_LONG_MSG) {
      task_t* current_task = get_current_task();

      size_t level = long_msg_level(msg->long_msg.msg_size);
      return is_valid_long_msg(current_task, msg->long_msg.table_handle, level);
    }

    return false;
  }

  bool ipc_send(task_t* receiver, memory::user_address_t msg_addr) {
    assert(receiver != nullptr);
    assert(receiver->state != TASK_STATE_UNUSED);

    task_t* sender = get_current_task();
    if (sender == receiver) [[unlikely]] {
      return false;
    }

    memory::mapped_address_t root_page_table = get_root_page_table(sender);

    if (!memory::is_user_page(root_page_table, msg_addr.page())) [[unlikely]] {
      return false;
    }

    msg_t* msg = memory::get_mapped_address(root_page_table, memory::virtual_address_t::from(msg_addr.page().value)).add(msg_addr.page_offset()).as<msg_t>();

    if (!is_valid_msg(sender, msg)) [[unlikely]] {
      return false;
    }

    msg->flags &= ~(MSG_FLAG_CANCELED);
    msg->flags &= ~(MSG_FLAG_FASTPATH);

    if (receiver->state == TASK_STATE_WAITING && receiver->prev_waiting_queue == null_tid && receiver->next_sender_task == null_tid) {
      return ipc_fastpath(receiver, sender, msg);
    }

    if (receiver->next_sender_task == null_tid) {
      receiver->next_sender_task = sender->tid;
      sender->prev_waiting_queue = receiver->tid;
    } else {
      task_t* last_waiting_task = lookup(receiver->next_sender_task);
      while (last_waiting_task->next_waiting_queue != null_tid) {
        last_waiting_task = lookup(last_waiting_task->next_waiting_queue);
      }
      last_waiting_task->next_waiting_queue = sender->tid;
      sender->prev_waiting_queue            = last_waiting_task->tid;
    }

    sender->msg = memory::mapped_address_t::from(msg);

    wait();

    return true;
  }

  bool ipc_nb_send(task_t* receiver, memory::user_address_t msg_addr) {
    assert(receiver != nullptr);
    assert(receiver->state != TASK_STATE_UNUSED);

    task_t* sender = get_current_task();
    if (sender == receiver) [[unlikely]] {
      return false;
    }

    memory::mapped_address_t root_page_table = get_root_page_table(sender);

    if (!memory::is_user_page(root_page_table, msg_addr.page())) [[unlikely]] {
      return false;
    }

    msg_t* msg = memory::get_mapped_address(root_page_table, memory::virtual_address_t::from(msg_addr.page().value)).add(msg_addr.page_offset()).as<msg_t>();

    if (!is_valid_msg(sender, msg)) [[unlikely]] {
      return false;
    }

    msg->flags &= ~(MSG_FLAG_CANCELED);
    msg->flags &= ~(MSG_FLAG_FASTPATH);

    if (receiver->state == TASK_STATE_WAITING && receiver->prev_waiting_queue == null_tid && receiver->next_sender_task == null_tid) {
      msg->flags &= ~(MSG_FLAG_CANCELED);
      msg->flags &= ~(MSG_FLAG_FASTPATH);
      return ipc_fastpath(receiver, sender, msg);
    }

    return false;
  }

  bool ipc_receive(memory::user_address_t msg_addr) {
    task_t* receiver = get_current_task();

    memory::mapped_address_t root_page_table = get_root_page_table(receiver);

    if (!memory::is_user_page(root_page_table, msg_addr.page())) [[unlikely]] {
      return false;
    }

    memory::mapped_address_t msg_page = memory::get_mapped_address(root_page_table, memory::virtual_address_t::from(msg_addr.page().value));
    msg_t*                   msg      = msg_page.add(msg_addr.page_offset()).as<msg_t>();
    receiver->msg                     = memory::mapped_address_t::from(msg);

    while (receiver->next_sender_task == null_tid) {
      wait();
    }

    receiver->msg = memory::mapped_address_t::null();

    task_t* sender             = lookup(receiver->next_sender_task);
    receiver->next_sender_task = sender->next_sender_task;

    if ((msg->flags & MSG_FLAG_FASTPATH) == 0) {
      move_msg(msg, sender, sender->msg.as<msg_t>());
      sender->msg = memory::mapped_address_t::null();
      if (sender->next_waiting_queue != null_tid) {
        task_t* next_sender             = lookup(sender->next_waiting_queue);
        next_sender->prev_waiting_queue = receiver->tid;
        receiver->next_waiting_queue    = next_sender->tid;
      }

      sender->prev_waiting_queue = null_tid;
      sender->next_waiting_queue = null_tid;

      wakeup(sender);
    }

    msg->flags &= ~MSG_FLAG_FASTPATH;

    return true;
  }

  void ipc_cancel(task_t* task) {
    assert(task->state == TASK_STATE_WAITING);

    if (task->next_sender_task != null_tid) {
      task_t* sender = lookup(task->next_sender_task);
      while (sender != nullptr) {
        task->next_sender_task = sender->next_waiting_queue;
        wakeup(sender);
      }
    }
    if (task->prev_waiting_queue != null_tid) {
      task_t* prev_sender = lookup(task->prev_waiting_queue);
      if (prev_sender->next_sender_task != null_tid) {
        prev_sender->next_sender_task = task->next_waiting_queue;
      } else {
        prev_sender->next_waiting_queue = task->next_waiting_queue;
      }
    }
    if (task->next_waiting_queue != null_tid) {
      task_t* next_sender             = lookup(task->next_waiting_queue);
      next_sender->prev_waiting_queue = task->prev_waiting_queue;
    }
  }
} // namespace caprese::task
