/**
 * @file task.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <caprese/task/task.h>

namespace caprese::task {
  task_t* create_task() {
    return nullptr;
  }
  void switch_to(const task_t* task) {
    (void)task;
  }
  task_t* lookup(tid_t tid) {
    (void)tid;
    return nullptr;
  }
} // namespace caprese::task
