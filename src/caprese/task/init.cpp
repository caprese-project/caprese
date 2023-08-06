#include <caprese/arch/memory.h>
#include <caprese/task/init.h>

namespace caprese::task {
  void init_task_space() { }

  task_t* create_kernel_task(void (*entry)(arch::boot_info_t*), arch::boot_info_t* boot_info) {
    (void)entry;
    (void)boot_info;
    return nullptr;
  }

  void load_init_task_payload(task_t* init_task, arch::boot_info_t* boot_info) {
    (void)init_task;
    (void)boot_info;
  }
} // namespace caprese::task
