#include <caprese/capability/init.h>

namespace caprese::capability {
  void init_capability_space() {}
  void create_init_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info) {
    (void)kernel_task;
    (void)boot_info;
  }
} // namespace caprese::capability
