#ifndef CAPRESE_CAPABILITY_INIT_H_
#define CAPRESE_CAPABILITY_INIT_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::capability {
  constexpr uint16_t NULL_CAP_CCID   = 0;
  constexpr uint16_t MEMORY_CAP_CCID = 1;
  constexpr uint16_t TASK_CAP_CCID   = 2;

  [[nodiscard]] bool create_builtin_capability_classes();
  [[nodiscard]] bool create_builtin_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info);
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_INIT_H_
