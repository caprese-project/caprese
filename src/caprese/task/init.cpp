#include <cstdlib>

#include <caprese/memory/page.h>
#include <caprese/task/init.h>

namespace caprese::task {
  void init_task_space() {
    auto root_page_table = memory::get_kernel_root_page_table();
    auto page            = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));

    memory::map(root_page_table,
                memory::virtual_address_t::from(CONFIG_TASK_SPACE_BASE),
                page.physical_address(),
                { .readable = true, .writable = true, .executable = false, .user = false },
                true);
    for (task_t* task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE); task < reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE + arch::PAGE_SIZE); ++task) {
      *task                = {};
      task->tid.index      = task - reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE);
      task->tid.generation = INIT_TID_GENERATION;
      task->flags |= TASK_FLAG_UNUSED;
    }
  }

  task_t* create_kernel_task(void (*entry)(arch::boot_info_t*), arch::boot_info_t* boot_info) {
    task_t* kernel_task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE);
    if ((kernel_task->flags & TASK_FLAG_UNUSED) == 0) [[unlikely]] {
      return nullptr;
    }

    kernel_task->tid.generation++;
    kernel_task->flags &= ~TASK_FLAG_UNUSED;
    kernel_task->flags |= TASK_FLAG_CREATING;
    arch::create_kernel_task(&kernel_task->arch_task, entry, boot_info);

    return kernel_task;
  }

  void switch_to_kernel_task(task_t* kernel_task) {
    arch::load_context(&kernel_task->arch_task);
  }

  void load_init_task_payload(task_t* init_task, arch::boot_info_t* boot_info) {
    (void)init_task;
    (void)boot_info;
  }
} // namespace caprese::task
