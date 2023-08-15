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
#include <cstdlib>

#include <caprese/memory/cls.h>
#include <caprese/memory/page.h>
#include <caprese/task/task.h>
#include <caprese/util/panic.h>

namespace caprese::task {
  task_t* create_task() {
    task_t* new_task = nullptr;

    auto root_page_table = get_kernel_root_page_table();
    for (uintptr_t page = CONFIG_TASK_SPACE_BASE; page < CONFIG_TASK_SPACE_BASE + CONFIG_TASK_SPACE_SIZE; page += arch::PAGE_SIZE) {
      if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) {
        memory::mapped_address_t physical_page = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
        if (physical_page.is_null()) [[unlikely]] {
          return nullptr;
        }

        auto result = memory::map(root_page_table,
                                  memory::virtual_address_t::from(page),
                                  physical_page.physical_address(),
                                  { .readable = true, .writable = true, .executable = false, .user = false },
                                  true);
        if (!result) [[unlikely]] {
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
          new_task = task;
          goto task_found;
        }
      }
    }

    return nullptr;

  task_found:

    new_task->flags &= ~TASK_FLAG_UNUSED;
    new_task->flags |= TASK_FLAG_CREATING;
    new_task->tid.generation++;

    bool result = arch::init_task(&new_task->arch_task, CONFIG_STACK_SPACE_BASE + new_task->tid.index * arch::PAGE_SIZE);
    if (!result) [[unlikely]] {
      new_task->tid.generation--;
      new_task->flags &= ~TASK_FLAG_CREATING;
      new_task->flags |= TASK_FLAG_UNUSED;
      return nullptr;
    }

    return new_task;
  }

  void switch_to(task_t* task) {
    if ((task->flags & TASK_FLAG_READY) == 0) [[unlikely]] {
      panic("Tried to switch to a non-ready task.");
    }
    task_t* current_task = get_current_task();
    current_task->flags &= ~TASK_FLAG_RUNNING;
    current_task->flags |= TASK_FLAG_READY;
    task->flags &= ~TASK_FLAG_READY;
    task->flags |= TASK_FLAG_RUNNING;
    memory::get_cls()->current_tid = task->tid;
    arch::switch_context(&current_task->arch_task, &task->arch_task);
  }

  task_t* lookup(tid_t tid) {
    task_t*   task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE) + tid.index;
    uintptr_t page = reinterpret_cast<uintptr_t>(task) & ~(arch::PAGE_SIZE - 1);

    if (tid.index == 0) [[unlikely]] {
      return task;
    }

    memory::mapped_address_t root_page_table = get_kernel_root_page_table();
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }
    if (task->flags & TASK_FLAG_UNUSED) [[unlikely]] {
      return nullptr;
    }

    return task;
  }

  task_t* get_current_task() {
    return lookup(memory::get_cls()->current_tid);
  }

  task_t* get_kernel_task() {
    return lookup({ .index = 0, .generation = 0 });
  }

  memory::mapped_address_t get_root_page_table(task_t* task) {
    return memory::physical_address_t::from(arch::get_root_page_table(&task->arch_task)).mapped_address();
  }

  memory::mapped_address_t get_kernel_root_page_table() {
    return task::get_root_page_table(task::get_kernel_task());
  }

  capability::capability_t* lookup_capability(task_t* task, capability::cid_t cid) {
    capability::capability_t* capability = capability::lookup(cid);
    if (capability == nullptr) [[unlikely]] {
      return nullptr;
    }
    if (capability->tid != std::bit_cast<uint32_t>(task->tid)) [[unlikely]] {
      return nullptr;
    }
    return capability;
  }
} // namespace caprese::task
