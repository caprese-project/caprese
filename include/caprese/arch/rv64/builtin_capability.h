#ifndef CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_
#define CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_

#include <cstdint>

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::arch::inline rv64 {
  constexpr uint16_t TRAP_CAP_CCID = 3;

  [[nodiscard]] bool create_builtin_capability_classes();
  [[nodiscard]] bool create_builtin_capabilities(task::task_t* kernel_task, const boot_info_t* boot_info);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_
