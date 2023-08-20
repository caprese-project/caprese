#include <bit>
#include <cstdlib>

#include <caprese/arch/rv64/builtin_capability.h>
#include <caprese/capability/capability.h>
#include <caprese/memory/heap.h>
#include <caprese/memory/page.h>
#include <caprese/task/task.h>

namespace caprese::arch::inline rv64 {
  bool create_builtin_capability_classes() {
    return true;
  }

  bool create_builtin_capabilities(task::task_t* kernel_task, const boot_info_t* boot_info) {
    (void)kernel_task;
    (void)boot_info;
    return true;
  }
} // namespace caprese::arch::inline rv64
