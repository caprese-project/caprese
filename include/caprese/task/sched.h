#ifndef CAPRESE_TASK_SCHED_H_
#define CAPRESE_TASK_SCHED_H_

#include <caprese/task/types.h>

namespace caprese::task {
  void               switch_to(task_t* task);
  void               yield();
  void               wait();
  void               wakeup(task_t* task);
  void               reschedule();
  void               insert_into_ready_queue(task_t* task);
  void               remove_from_ready_queue(task_t* task);
  void               insert_into_killed_queue(task_t* task);
  [[nodiscard]] bool schedule();
} // namespace caprese::task

#endif // CAPRESE_TASK_SCHED_H_
