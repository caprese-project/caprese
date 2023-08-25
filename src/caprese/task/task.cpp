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
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <caprese/memory/cls.h>
#include <caprese/memory/heap.h>
#include <caprese/memory/kernel_space.h>
#include <caprese/memory/page.h>
#include <caprese/task/cap.h>
#include <caprese/task/ipc.h>
#include <caprese/task/sched.h>
#include <caprese/task/task.h>
#include <caprese/util/panic.h>

namespace caprese::task {
  task_t* create_task() {
    task_t* new_task = nullptr;

    memory::mapped_address_t kernel_root_page_table = get_kernel_root_page_table();
    for (uintptr_t page = CONFIG_TASK_SPACE_BASE; page < CONFIG_TASK_SPACE_BASE + CONFIG_TASK_SPACE_SIZE; page += arch::PAGE_SIZE) {
      if (!memory::is_mapped(kernel_root_page_table, memory::virtual_address_t::from(page))) {
        memory::mapped_address_t physical_page = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
        if (physical_page.is_null()) [[unlikely]] {
          return nullptr;
        }

        bool result = memory::map(kernel_root_page_table,
                                  memory::virtual_address_t::from(page),
                                  physical_page.physical_address(),
                                  { .readable = true, .writable = true, .executable = false, .user = false, .global = true },
                                  true);
        if (!result) [[unlikely]] {
          return nullptr;
        }

        for (task_t* task = reinterpret_cast<task_t*>(page); task < reinterpret_cast<task_t*>(page + arch::PAGE_SIZE); ++task) {
          *task                = {};
          task->tid.index      = (page - CONFIG_TASK_SPACE_BASE) / sizeof(task_t) + (task - reinterpret_cast<task_t*>(page));
          task->tid.generation = INIT_TID_GENERATION;
          task->state          = TASK_STATE_UNUSED;
        }
      }
      for (task_t* task = reinterpret_cast<task_t*>(page); task < reinterpret_cast<task_t*>(page + arch::PAGE_SIZE); ++task) {
        if (task->state == TASK_STATE_UNUSED) {
          new_task = task;
          goto task_found;
        }
      }
    }

    return nullptr;

  task_found:

    new_task->state = TASK_STATE_CREATING;
    new_task->tid.generation++;
    new_task->free_cap_list        = 0;
    new_task->used_cap_space_count = 0;
    new_task->prev_ready_task      = null_tid;
    new_task->next_ready_task      = null_tid;
    new_task->prev_waiting_queue   = null_tid;
    new_task->next_waiting_queue   = null_tid;
    new_task->next_sender_task     = null_tid;
    new_task->msg                  = {};

    memory::mapped_address_t root_page_table = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
    if (root_page_table.is_null()) [[unlikely]] {
      kill(new_task);
      return nullptr;
    }
    memset(root_page_table.as<void>(), 0, arch::PAGE_SIZE);

    bool result = memory::copy_kernel_space_mapping(root_page_table, task::get_kernel_root_page_table());
    if (!result) [[unlikely]] {
      kill(new_task);
      return nullptr;
    }

    memory::mapped_address_t stack = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
    if (stack.is_null()) [[unlikely]] {
      kill(new_task);
      return nullptr;
    }

    result = memory::map(root_page_table,
                         memory::virtual_address_t::from(CONFIG_STACK_SPACE_BASE + new_task->tid.index * arch::PAGE_SIZE),
                         stack.physical_address(),
                         { .readable = true, .writable = true, .executable = false, .user = false, .global = true },
                         true);
    if (!result) [[unlikely]] {
      free(stack.as<void>());
      kill(new_task);
      return nullptr;
    }

    result = arch::init_task(&new_task->arch_task, CONFIG_STACK_SPACE_BASE + new_task->tid.index * arch::PAGE_SIZE);
    if (!result) [[unlikely]] {
      kill(new_task);
      return nullptr;
    }

    arch::set_root_page_table(&new_task->arch_task, root_page_table.physical_address().value);

    return new_task;
  }

  void kill(task_t* task) {
    assert(task->state != TASK_STATE_UNUSED);

    if (task->state == TASK_STATE_READY) {
      remove_from_ready_queue(task);
    } else if (task->state == TASK_STATE_WAITING) {
      ipc_cancel(task);
    }

    task->state = TASK_STATE_KILLED;
    insert_into_killed_queue(task);
    if (task == get_current_task()) {
      reschedule();
    }
  }

  void cleanup(task_t* task) {
    assert(task->state == TASK_STATE_KILLED);
    assert(task != get_current_task());

    memory::mapped_address_t root_page_table = get_root_page_table(task);

    memory::virtual_address_t stack        = memory::virtual_address_t::from(CONFIG_STACK_SPACE_BASE + task->tid.index * arch::PAGE_SIZE);
    memory::mapped_address_t  mapped_stack = memory::get_mapped_address(root_page_table, stack);
    if (!mapped_stack.is_null()) {
      if (!memory::unmap(root_page_table, stack)) [[unlikely]] {
        panic("Failed to unmap stack.");
      }
      memory::deallocate(mapped_stack);
    }

    task_t* init_task = lookup({ .index = 1, .generation = 0 });

    size_t cap_list_size = allocated_cap_list_size(task);
    for (cid_handle_t handle = 0; handle < cap_list_size; ++handle) {
      move_capability(init_task, task, handle);
    }

    size_t cap_list_pages = (cap_list_size + arch::PAGE_SIZE - 1) / arch::PAGE_SIZE;
    for (uintptr_t offset = 0; offset < cap_list_pages; offset += arch::PAGE_SIZE) {
      memory::virtual_address_t va = memory::virtual_address_t::from(CONFIG_CAPABILITY_LIST_SPACE_BASE + offset);
      memory::mapped_address_t  ma = memory::get_mapped_address(root_page_table, va);
      if (!memory::unmap(root_page_table, va)) [[unlikely]] {
        panic("Failed to unmap cap list.");
      }
      memory::deallocate(ma);
    }

    // TODO: unmap task block
  }

  task_t* lookup(tid_t tid) {
    if (tid == null_tid) [[unlikely]] {
      return nullptr;
    }

    task_t*   task = reinterpret_cast<task_t*>(CONFIG_TASK_SPACE_BASE) + tid.index;
    uintptr_t page = reinterpret_cast<uintptr_t>(task) & ~(arch::PAGE_SIZE - 1);

    if (tid.index == 0) [[unlikely]] {
      return task;
    }

    memory::mapped_address_t root_page_table = get_kernel_root_page_table();
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }
    if (task->state == TASK_STATE_UNUSED) [[unlikely]] {
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
} // namespace caprese::task
