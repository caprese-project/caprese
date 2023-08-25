#include <cassert>
#include <utility>

#include <caprese/memory/cls.h>
#include <caprese/task/sched.h>
#include <caprese/task/task.h>
#include <caprese/util/panic.h>

namespace caprese::task {
  task_t* ready_queue_head  = nullptr;
  task_t* ready_queue_tail  = nullptr;
  task_t* killed_queue_head = nullptr;

  void switch_to(task_t* task) {
    if (task->state != TASK_STATE_READY && task->state != TASK_STATE_CREATING) [[unlikely]] {
      panic("Attempted to switch to a non-ready task.");
    }

    if (task->state == TASK_STATE_READY) {
      remove_from_ready_queue(task);
    }

    task_t* current_task = get_current_task();

    task->state                    = TASK_STATE_RUNNING;
    memory::get_cls()->current_tid = task->tid;

    if (current_task == nullptr) [[unlikely]] {
      arch::load_context(&task->arch_task);
      std::unreachable();
    }

    if (current_task->state == TASK_STATE_RUNNING) {
      insert_into_ready_queue(current_task);
    }

    arch::switch_context(&current_task->arch_task, &task->arch_task);
  }

  void yield() {
    task_t* task = get_current_task();
    assert(task->state == TASK_STATE_RUNNING);
    insert_into_ready_queue(task);
    reschedule();
  }

  void wait() {
    task_t* task = get_current_task();
    assert(task->state == TASK_STATE_RUNNING);
    task->state = TASK_STATE_WAITING;
    reschedule();
  }

  void wakeup(task_t* task) {
    if (task->state != TASK_STATE_WAITING) [[unlikely]] {
      panic("Attempted to wake up a task that is not waiting.");
    }
    insert_into_ready_queue(task);
  }

  void reschedule() {
    switch_to(get_kernel_task());
  }

  void insert_into_ready_queue(task_t* task) {
    assert(task->state != TASK_STATE_READY);

    task->state = TASK_STATE_READY;

    if (ready_queue_head == nullptr) [[unlikely]] {
      assert(ready_queue_tail == nullptr);
      ready_queue_head = task;
      ready_queue_tail = task;
    } else {
      task_t* tail_task          = ready_queue_tail;
      tail_task->next_ready_task = task->tid;
      task->prev_ready_task      = tail_task->tid;
      ready_queue_tail           = task;
    }
  }

  void remove_from_ready_queue(task_t* task) {
    if (task->prev_ready_task == null_tid) {
      assert(ready_queue_head == task);
      ready_queue_head = lookup(task->next_ready_task);
      if (ready_queue_head != nullptr) {
        ready_queue_head->prev_ready_task = null_tid;
      } else {
        ready_queue_tail = nullptr;
      }
    } else if (task->next_ready_task == null_tid) {
      assert(ready_queue_tail == task);
      ready_queue_tail = lookup(task->prev_ready_task);
      if (ready_queue_tail != nullptr) {
        ready_queue_tail->next_ready_task = null_tid;
      } else {
        ready_queue_head = nullptr;
      }
    } else {
      task_t* prev_ready_task          = lookup(task->prev_ready_task);
      task_t* next_ready_task          = lookup(task->next_ready_task);
      prev_ready_task->next_ready_task = task->next_ready_task;
      next_ready_task->prev_ready_task = task->prev_ready_task;
    }
  }

  void insert_into_killed_queue(task_t* task) {
    assert(task->state == TASK_STATE_KILLED);
    task->next_ready_task = null_tid;
    if (killed_queue_head != nullptr) {
      task->next_ready_task = killed_queue_head->tid;
    }
    killed_queue_head = task;
  }

  bool schedule() {
    assert(get_current_task() == get_kernel_task());

    if (killed_queue_head != nullptr) [[unlikely]] {
      task_t* next_killed_queue = lookup(killed_queue_head->next_ready_task);
      cleanup(killed_queue_head);
      killed_queue_head = next_killed_queue;
    }

    if (ready_queue_head == nullptr) [[unlikely]] {
      return false;
    }

    switch_to(ready_queue_head);

    return true;
  }
} // namespace caprese::task
