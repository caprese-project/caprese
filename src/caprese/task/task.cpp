/**
 * @file task.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *Z
 */
#include <cstdio>
#include <cstdlib>

#include <caprese/memory/page.h>
#include <caprese/task/task.h>

namespace caprese::task {
  task_t* create_task() {
    auto root_page_table = memory::get_kernel_root_page_table();
    for (uintptr_t page = CONFIG_TASK_SPACE_BASE; page < CONFIG_TASK_SPACE_BASE + CONFIG_TASK_SPACE_SIZE; page += arch::PAGE_SIZE) {
      if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) {
        printf("not mapped\n");
        auto result = memory::map(root_page_table,
                                  memory::virtual_address_t::from(page),
                                  memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE)).physical_address(),
                                  { .readable = 1, .writable = 1, .executable = 0, .user = 0 },
                                  true);
        if (!result) [[unlikely]] {
          printf("Failed to map\n");
          return nullptr;
        }

        for (task_t* task = reinterpret_cast<task_t*>(page); task < reinterpret_cast<task_t*>(page + arch::PAGE_SIZE); ++task) {
          *task                = {};
          task->tid.index      = (page - CONFIG_TASK_SPACE_BASE) / sizeof(task_t) + (task - reinterpret_cast<task_t*>(page));
          task->tid.generation = INIT_TID_GENERATION;
          task->flags |= TASK_FLAG_UNUSED;
        }
      }
      for (task_t* task = reinterpret_cast<task_t*>(page); task < reinterpret_cast<task_t*>(page + arch::PAGE_SIZE); ++task) {
        if (task->flags & TASK_FLAG_UNUSED) {
          task->flags &= ~TASK_FLAG_UNUSED;
          task->flags |= TASK_FLAG_CREATING;
          task->tid.generation++;
          return task;
        }
      }
    }

    return nullptr;
  }

  void switch_to(task_t* task) {
    arch::switch_context(&get_current_task()->arch_task, &task->arch_task);
  }

  task_t* lookup(tid_t tid) {
    (void)tid;
    return nullptr;
  }

  task_t* get_current_task() {
    return nullptr;
  }
} // namespace caprese::task
