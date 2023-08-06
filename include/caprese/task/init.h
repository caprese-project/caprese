#ifndef CAPRESE_TASK_INIT_H_
#define CAPRESE_TASK_INIT_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::task {
  void    init_task_space();
  task_t* create_kernel_task(void (*entry)(arch::boot_info_t*), arch::boot_info_t* boot_info);
  void    load_init_task_payload(task_t* init_task, arch::boot_info_t* boot_info);
} // namespace caprese::task

#endif // CAPRESE_TASK_INIT_H_
