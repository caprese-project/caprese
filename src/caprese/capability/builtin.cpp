#include <caprese/arch/builtin_capability.h>
#include <caprese/capability/builtin.h>

namespace caprese::capability {
  bool create_builtin_capability_classes() {
    return arch::create_builtin_capability_classes();
  }

  bool create_builtin_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info) {
    return arch::create_builtin_capabilities(kernel_task, boot_info);
  }
} // namespace caprese::capability
