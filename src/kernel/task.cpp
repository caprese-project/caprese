#include <cassert>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <iterator>
#include <mutex>
#include <utility>

#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <log/log.h>

namespace {
  constexpr const char* tag = "kernel/task";

  spinlock_t next_tid_lock;
  spinlock_t lookup_tid_lock;
  uint32_t   cur_tid = 0;
  jmp_buf    jump_buffer;

  task_queue_t         ready_queue;
  recursive_spinlock_t ready_queue_lock;

  tid_t next_tid() {
    std::lock_guard lock(next_tid_lock);
    ++cur_tid;
    return std::bit_cast<tid_t>(cur_tid);
  }

  void push_ready_queue(map_ptr<task_t> task) {
    assert(task != nullptr);
    assert(task->state == task_state_t::ready);
    assert(task->prev_ready_task == nullptr);
    assert(task->next_ready_task == nullptr);

    std::lock_guard lock(ready_queue_lock);

    if (ready_queue.head == nullptr) {
      ready_queue.head = task;
      ready_queue.tail = task;
    } else {
      ready_queue.tail->next_ready_task = task;
      ready_queue.tail                  = task;
    }
  }

  void remove_ready_queue(map_ptr<task_t> task) {
    assert(task != nullptr);
    assert(task->state == task_state_t::ready);

    std::lock_guard lock(ready_queue_lock);

    if (ready_queue.head == task) {
      ready_queue.head = task->next_ready_task;
    } else {
      task->prev_ready_task->next_ready_task = task->next_ready_task;
    }

    if (ready_queue.tail == task) {
      ready_queue.tail = task->prev_ready_task;
    } else {
      task->next_ready_task->prev_ready_task = task->prev_ready_task;
    }

    task->prev_ready_task = 0_map;
    task->next_ready_task = 0_map;
  }

  map_ptr<task_t> pop_ready_task() {
    std::lock_guard lock(ready_queue_lock);

    map_ptr<task_t> task = ready_queue.head;
    if (task == nullptr) [[unlikely]] {
      return 0_map;
    }

    remove_ready_queue(task);
    return task;
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
      queue.tail                    = task;
    }
  }

  void remove_waiting_task(task_queue_t& queue, map_ptr<task_t> task) {
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
} // namespace

void init_task(map_ptr<task_t> task, map_ptr<cap_space_t> cap_space, map_ptr<page_table_t> root_page_table, map_ptr<page_table_t> (&cap_space_page_tables)[NUM_INTER_PAGE_TABLE + 1]) {
  assert(task != nullptr);
  assert(cap_space != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  task->tid = next_tid();

  std::lock_guard lock(task->lock);

  task->cap_count         = {};
  task->prev_ready_task   = 0_map;
  task->next_ready_task   = 0_map;
  task->prev_waiting_task = 0_map;
  task->next_waiting_task = 0_map;
  task->free_slots        = 0_map;
  task->root_page_table   = root_page_table;
  task->state             = task_state_t::suspended;

  memset(root_page_table.get(), 0, sizeof(page_table_t));

  constexpr size_t max_page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(max_page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += max_page_size) {
    map_ptr<pte_t> pte = root_page_table->walk(make_virt_ptr(CONFIG_MAPPED_SPACE_BASE + phys), MAX_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 0, .global = 1 });
    pte->set_next_page(make_phys_ptr(phys));
    pte->enable();
  }

  for (auto& table : cap_space_page_tables) {
    memset(table.get(), 0, sizeof(page_table_t));
  }

  map_ptr<page_table_t> page_table = root_page_table;
  map_ptr<pte_t>        pte        = 0_map;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level >= GIGA_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(make_virt_ptr(CONFIG_CAPABILITY_SPACE_BASE), level);
    assert(pte->is_disabled());
    pte->set_next_page(cap_space_page_tables[level - 1].as<void>());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 0, .user = 0, .global = 0 });
    pte->enable();
    page_table = pte->get_next_page().as<page_table_t>();
  }
  if (!extend_cap_space(task, cap_space_page_tables[0].as<void>())) {
    panic("Failed to extend cap space.");
  }

  if (!insert_cap_space(task, cap_space)) [[unlikely]] {
    panic("Failed to insert cap space.");
  }

  arch_init_task(task, return_to_user_mode);
}

void init_idle_task(map_ptr<task_t> task, map_ptr<page_table_t> root_page_table) {
  assert(task != nullptr);
  assert(root_page_table != nullptr);

  assert(task->state == task_state_t::unused);

  std::lock_guard lock(task->lock);

  task->cap_count         = {};
  task->prev_ready_task   = 0_map;
  task->next_ready_task   = 0_map;
  task->prev_waiting_task = 0_map;
  task->next_waiting_task = 0_map;
  task->free_slots        = 0_map;
  task->root_page_table   = root_page_table;
  task->state             = task_state_t::ready;

  memset(root_page_table.get(), 0, sizeof(page_table_t));

  constexpr size_t max_page_size = get_page_size(MAX_PAGE_TABLE_LEVEL);
  static_assert(std::countr_zero(CONFIG_MAPPED_SPACE_BASE) >= std::countr_zero(max_page_size));
  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += max_page_size) {
    map_ptr<pte_t> pte = root_page_table->walk(make_virt_ptr(CONFIG_MAPPED_SPACE_BASE + phys), MAX_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 0, .global = 1 });
    pte->set_next_page(make_phys_ptr(phys));
    pte->enable();
  }

  arch_init_task(task, idle);
}

map_ptr<cap_slot_t> insert_cap(map_ptr<task_t> task, capability_t cap) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  if (task->free_slots == nullptr) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_slot_t> slot = task->free_slots;
  task->free_slots         = slot->prev;

  slot->prev = 0_map;
  slot->next = 0_map;
  slot->cap  = cap;

  return slot;
}

map_ptr<cap_slot_t> transfer_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot) {
  assert(task != nullptr);
  assert(src_slot != nullptr);

  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::scoped_lock lock { src_task->lock, task->lock };

  if (get_cap_type(src_slot->cap) == CAP_NULL || get_cap_type(src_slot->cap) == CAP_ZOMBIE) [[unlikely]] {
    return 0_map;
  }

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  if (src_task->state == task_state_t::unused || src_task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_slot_t> dst_slot = insert_cap(task, src_slot->cap);

  if (dst_slot == nullptr) [[unlikely]] {
    return 0_map;
  }

  dst_slot->prev = src_slot->prev;
  dst_slot->next = src_slot->next;

  src_slot->cap        = make_null_cap();
  src_slot->prev       = src_task->free_slots;
  src_slot->next       = 0_map;
  src_task->free_slots = src_slot;

  return dst_slot;
}

map_ptr<cap_slot_t> delegate_cap(map_ptr<task_t> task, map_ptr<cap_slot_t> src_slot) {
  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::scoped_lock lock { src_task->lock, task->lock };

  if (get_cap_type(src_slot->cap) == CAP_NULL || get_cap_type(src_slot->cap) == CAP_ZOMBIE) [[unlikely]] {
    return 0_map;
  }

  if (task->state == task_state_t::unused || task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  if (src_task->state == task_state_t::unused || src_task->state == task_state_t::killed) [[unlikely]] {
    return 0_map;
  }

  map_ptr<cap_slot_t> dst_slot = insert_cap(task, src_slot->cap);

  if (dst_slot == nullptr) [[unlikely]] {
    return 0_map;
  }

  dst_slot->prev = src_slot;
  dst_slot->next = src_slot->next;
  src_slot->cap  = make_null_cap();
  src_slot->next = dst_slot;

  return dst_slot;
}

map_ptr<cap_slot_t> copy_cap(map_ptr<cap_slot_t> src_slot) {
  assert(src_slot != nullptr);

  map_ptr<task_t>& src_task = src_slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(src_task->lock);

  map_ptr<cap_slot_t> dst_slot = 0_map;

  switch (get_cap_type(src_slot->cap)) {
    case CAP_NULL:
      break;
    case CAP_MEM:
      break;
    case CAP_TASK:
      dst_slot = insert_cap(src_task, src_slot->cap);
      break;
    case CAP_ENDPOINT:
      dst_slot = insert_cap(src_task, src_slot->cap);
      break;
    case CAP_PAGE_TABLE:
      break;
    case CAP_VIRT_PAGE:
      break;
    case CAP_CAP_SPACE:
      break;
    case CAP_ZOMBIE:
      break;
    case CAP_UNKNOWN:
      break;
  }

  if (dst_slot == nullptr) [[unlikely]] {
    return 0_map;
  }

  dst_slot->prev = src_slot;
  dst_slot->next = src_slot->next;
  src_slot->next = dst_slot;

  return dst_slot;
}

[[nodiscard]] bool revoke_cap(map_ptr<cap_slot_t> slot) {
  assert(slot != nullptr);

  map_ptr<task_t>& task = slot->get_cap_space()->meta_info.task;

  std::lock_guard lock(task->lock);

  if (task->state == task_state_t::unused) [[unlikely]] {
    panic("Unexpected task state.");
  }

  map_ptr<cap_slot_t> cap_slot = slot;
  while (cap_slot->next != nullptr) {
    cap_slot = cap_slot->next;
  }

  while (cap_slot != slot) {
    // TODO: impl
    switch (get_cap_type(cap_slot->cap)) {
      case CAP_NULL:
        break;
      case CAP_MEM:
        break;
      case CAP_TASK:
        break;
      case CAP_ENDPOINT:
        break;
      case CAP_PAGE_TABLE:
        break;
      case CAP_VIRT_PAGE:
        break;
      case CAP_CAP_SPACE:
        break;
      case CAP_ZOMBIE:
        break;
      default:
        break;
    }

    cap_slot = cap_slot->prev;
  }

  return true;
}

void kill_task(map_ptr<task_t> task, int exit_status) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  assert(task->state != task_state_t::unused);

  switch (task->state) {
    case task_state_t::ready:
      remove_ready_queue(task);
      break;
    case task_state_t::waiting:
      // TODO: impl
      break;
    default:
      break;
  }

  // TODO: Release caps

  task->state       = task_state_t::killed;
  task->exit_status = exit_status;

  if (task->tid.index == 1) [[unlikely]] {
    panic("The root task has been killed.");
  }

  logd(tag, "Task 0x%x has been killed.", task->tid);

  if (task == get_cls()->current_task) [[unlikely]] {
    resched();
    std::unreachable();
  }
}

void switch_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  map_ptr<task_t> old_task = get_cls()->current_task;

  if (task == old_task) [[unlikely]] {
    return;
  }

  {
    std::lock_guard lock(task->lock);
    if (task->state != task_state_t::ready) [[unlikely]] {
      return;
    }
    remove_ready_queue(task);
    task->state = task_state_t::running;
  }

  get_cls()->current_task = task;
  old_task->state         = task_state_t::ready;
  push_ready_queue(old_task);
  switch_context(make_map_ptr(&task->context), make_map_ptr(&old_task->context));
  assert(old_task->state == task_state_t::running);
  assert(get_cls()->current_task == old_task);
}

void suspend_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  switch (task->state) {
    case task_state_t::running:
      task->state = task_state_t::suspended;
      if (task == get_cls()->current_task) {
        resched();
      } else {
        // TODO: ipi
      }
      break;
    case task_state_t::ready:
      remove_ready_queue(task);
      task->state = task_state_t::suspended;
      break;
    case task_state_t::waiting:
      task->state = task_state_t::suspended;
      break;
    default:
      break;
  }
}

void resume_task(map_ptr<task_t> task) {
  assert(task != nullptr);

  std::lock_guard lock(task->lock);

  if (task->state != task_state_t::suspended) [[unlikely]] {
    return;
  }

  task->state = task_state_t::ready;
  push_ready_queue(task);
}

bool ipc_send_short(bool blocking, map_ptr<endpoint_t> endpoint, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5) {
  std::unique_lock ep_lock(endpoint->lock);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  if (endpoint->receiver_queue.head != nullptr && endpoint->sender_queue.head == nullptr) {
    map_ptr<task_t> receiver           = endpoint->receiver_queue.head;
    receiver->msg_buf.cap_part_length  = 0;
    receiver->msg_buf.data_part_length = 6;
    receiver->msg_buf.data[0]          = arg0;
    receiver->msg_buf.data[1]          = arg1;
    receiver->msg_buf.data[2]          = arg2;
    receiver->msg_buf.data[3]          = arg3;
    receiver->msg_buf.data[4]          = arg4;
    receiver->msg_buf.data[5]          = arg5;

    {
      std::lock_guard lock(receiver->lock);
      assert(endpoint->receiver_queue.head == receiver);
      remove_waiting_task(endpoint->receiver_queue, receiver);
      receiver->state = task_state_t::ready;
      push_ready_queue(receiver);
    }

    return true;
  }

  if (!blocking) {
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

  {
    std::lock_guard lock(cur_task->lock);
    cur_task->state = task_state_t::waiting;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  ep_lock.unlock();

  resched();

  ep_lock.lock();

  // The message should be written to receiver->msg_buf in ipc_receive.
  return true;
}

bool ipc_send_long(bool blocking, map_ptr<endpoint_t> endpoint) {
  std::unique_lock ep_lock(endpoint->lock);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  if (endpoint->receiver_queue.head != nullptr && endpoint->sender_queue.head == nullptr) {
    map_ptr<task_t> receiver = endpoint->receiver_queue.head;

    if (!ipc_transfer_msg(receiver, cur_task)) [[unlikely]] {
      return false;
    }

    {
      std::lock_guard lock(receiver->lock);
      assert(endpoint->receiver_queue.head == receiver);
      remove_waiting_task(endpoint->receiver_queue, receiver);
      receiver->state = task_state_t::ready;
      push_ready_queue(receiver);
    }

    return true;
  }

  if (!blocking) {
    return false;
  }

  {
    std::lock_guard lock(cur_task->lock);
    cur_task->state = task_state_t::waiting;
    push_waiting_queue(endpoint->sender_queue, cur_task);
  }

  ep_lock.unlock();

  resched();

  ep_lock.lock();

  // The message should be written to receiver->msg_buf in ipc_receive.
  return true;
}

bool ipc_receive(bool blocking, map_ptr<endpoint_t> endpoint) {
  std::unique_lock ep_lock(endpoint->lock);

  map_ptr<task_t> cur_task = get_cls()->current_task;

  if (endpoint->receiver_queue.head == nullptr && endpoint->sender_queue.head != nullptr) {
    map_ptr<task_t> sender = endpoint->sender_queue.head;

    if (!ipc_transfer_msg(cur_task, sender)) [[unlikely]] {
      return false;
    }

    {
      std::lock_guard lock(sender->lock);
      assert(endpoint->sender_queue.head == sender);
      remove_waiting_task(endpoint->sender_queue, sender);
      sender->state = task_state_t::ready;
      push_ready_queue(sender);
    }

    return true;
  }

  if (!blocking) {
    return false;
  }

  {
    std::lock_guard lock(cur_task->lock);
    cur_task->state = task_state_t::waiting;
    push_waiting_queue(endpoint->receiver_queue, cur_task);
  }

  ep_lock.unlock();

  resched();

  ep_lock.lock();

  // The message should be written to cur_task->msg_buf in ipc_send_xxx.
  return true;
}

bool ipc_transfer_msg(map_ptr<task_t> dst, map_ptr<task_t> src) {
  assert(dst != nullptr);
  assert(src != nullptr);

  const message_buffer_t& src_msg_buf = src->msg_buf;
  message_buffer_t&       dst_msg_buf = dst->msg_buf;

  for (size_t i = 0; i < src_msg_buf.cap_part_length; ++i) {
    map_ptr<cap_slot_t> src_slot = lookup_cap(src, src_msg_buf.data[i]);
    if (src_slot == nullptr) [[unlikely]] {
      return false;
    }

    map_ptr<cap_slot_t> dst_slot = delegate_cap(dst, src_slot);
    if (dst_slot == nullptr) [[unlikely]] {
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

extern "C" {
  static void lookup_tid_signal_handler([[maybe_unused]] int) {
    longjmp(jump_buffer, 1);
  }
}

map_ptr<task_t> lookup_tid(tid_t tid) {
  if (tid.index == 0) [[unlikely]] {
    return 0_map;
  }

  std::lock_guard lock(lookup_tid_lock);

  auto old_handler = signal(SIGSEGV, lookup_tid_signal_handler);
  if (old_handler == SIG_ERR) [[unlikely]] {
    panic("Failed to set signal handler.");
  }

  // If the task's area is not mapped, accessing it will cause a page fault, resulting in a SEGV.
  // The signal handler for SEGV is set to 'lookup_tid_signal_handler', which uses 'longjmp' to safely return control, subsequently setting 'task' to 'nullptr' in the 'else' branch of the 'setjmp' check.

  map_ptr<task_t> task = map_ptr<task_t>::from(CONFIG_TASK_SPACE_BASE) + tid.index;
  if (setjmp(jump_buffer) == 0) {
    if (task->state == task_state_t::unused) [[unlikely]] {
      task = 0_map;
    }
  } else {
    task = 0_map;
  }

  signal(SIGSEGV, old_handler);
  return task;
}

void resched() {
  map_ptr<task_t> cur_task  = get_cls()->current_task;
  map_ptr<task_t> idle_task = get_cls()->idle_task;
  get_cls()->current_task   = idle_task;
  switch_context(make_map_ptr(&idle_task->context), make_map_ptr(&cur_task->context));
}

void yield() {
  {
    map_ptr<task_t> cur_task = get_cls()->current_task;
    std::lock_guard lock(cur_task->lock);
    cur_task->state = task_state_t::ready;
    push_ready_queue(cur_task);
  }
  resched();
}

void idle() {
  while (true) {
    map_ptr<task_t> task = pop_ready_task();
    if (task == nullptr) {
      // TODO: wait for interrupt.
      continue;
    }
    get_cls()->current_task = task;
    task->state             = task_state_t::running;
    switch_context(make_map_ptr(&task->context), make_map_ptr(&get_cls()->idle_task->context));
  }
}
