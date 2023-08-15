#ifndef CAPRESE_CAPABILITY_INIT_H_
#define CAPRESE_CAPABILITY_INIT_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::capability {
  [[nodiscard]] bool create_builtin_capability_classes();
  [[nodiscard]] bool create_init_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info);
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_INIT_H_
