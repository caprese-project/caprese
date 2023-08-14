#include <caprese/arch/builtin_capability.h>
#include <caprese/capability/init.h>

namespace caprese::capability {
  void create_builtin_capability_classes() {
    arch::create_builtin_capability_classes();
  }

  void create_init_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info) {
    (void)kernel_task;
    (void)boot_info;
  }
} // namespace caprese::capability
