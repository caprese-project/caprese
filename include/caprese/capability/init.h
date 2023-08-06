#ifndef CAPRESE_CAPABILITY_INIT_H_
#define CAPRESE_CAPABILITY_INIT_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::capability {
  void init_capability_space();
  void create_init_capabilities(task::task_t* kernel_task, arch::boot_info_t* boot_info);
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_INIT_H_
