#include <caprese/arch/builtin_capability.h>
#include <caprese/capability/init.h>

namespace caprese::capability {
  bool create_builtin_capability_classes() {
    return arch::create_builtin_capability_classes();
  }

  bool create_init_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info) {
    (void)kernel_task;
    (void)boot_info;
    return true;
  }
} // namespace caprese::capability
