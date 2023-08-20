#ifndef CAPRESE_TASK_INIT_H_
#define CAPRESE_TASK_INIT_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::task {
  [[nodiscard]] bool    init_task_space();
  [[nodiscard]] task_t* create_kernel_task(void (*entry)(const arch::boot_info_t*), const arch::boot_info_t* boot_info);
  void                  switch_to_kernel_task(task_t* kernel_task);
  [[nodiscard]] bool    load_init_task_payload(cid_handle_t init_task_cid_handle, const arch::boot_info_t* boot_info);
} // namespace caprese::task

#endif // CAPRESE_TASK_INIT_H_
