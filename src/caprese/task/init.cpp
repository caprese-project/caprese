#include <cstdlib>

#include <caprese/arch/task.h>
#include <caprese/memory/cls.h>
#include <caprese/memory/page.h>
#include <caprese/task/init.h>

namespace caprese::task {
  bool init_task_space() {
    auto root_page_table = memory::get_current_root_page_table();
    auto page            = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));

    if (page.is_null()) [[unlikely]] {
      return false;
    }

    bool result = memory::map(root_page_table,
                              memory::virtual_address_t::from(CONFIG_TASK_SPACE_BASE),
                              page.physical_address(),
                              { .readable = true, .writable = true, .executable = false, .user = false, .global = true },
                              true);

    if (!result) [[unlikely]] {
      return false;
    }

    for (task_t* task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE); task < reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE + arch::PAGE_SIZE); ++task) {
      *task                = {};
      task->tid.index      = task - reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE);
      task->tid.generation = INIT_TID_GENERATION;
      task->flags |= TASK_FLAG_UNUSED;
    }

    return true;
  }

  task_t* create_kernel_task(void (*entry)(const arch::boot_info_t*), const arch::boot_info_t* boot_info) {
    task_t* kernel_task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE);
    if ((kernel_task->flags & TASK_FLAG_UNUSED) == 0) [[unlikely]] {
      return nullptr;
    }

    kernel_task->tid.generation++;
    kernel_task->flags &= ~TASK_FLAG_UNUSED;
    kernel_task->flags |= TASK_FLAG_CREATING;

    bool result = arch::create_kernel_task(&kernel_task->arch_task, entry, boot_info);
    if (!result) [[unlikely]] {
      kernel_task->tid.generation--;
      kernel_task->flags &= ~TASK_FLAG_CREATING;
      kernel_task->flags |= TASK_FLAG_UNUSED;
      return nullptr;
    }

    return kernel_task;
  }

  void switch_to_kernel_task(task_t* kernel_task) {
    kernel_task->flags &= ~TASK_FLAG_CREATING;
    kernel_task->flags |= TASK_FLAG_RUNNING;
    memory::get_cls()->current_tid = kernel_task->tid;
    arch::load_context(&kernel_task->arch_task);
  }

  bool load_init_task_payload(task_t* init_task, const arch::boot_info_t* boot_info) {
    bool result = arch::load_init_task_payload(&init_task->arch_task, boot_info);
    if (!result) [[unlikely]] {
      return false;
    }

    init_task->flags &= ~TASK_FLAG_CREATING;
    init_task->flags |= TASK_FLAG_READY;

    return true;
  }
} // namespace caprese::task
