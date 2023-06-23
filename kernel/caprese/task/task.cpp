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
 *
 */

#include <cstring>

#include <caprese/capability/capability.h>
#include <caprese/memory/page_stack.h>
#include <caprese/panic.h>
#include <caprese/task/task.h>

#include <libcaprese/util/enum.h>

namespace caprese::task {
  namespace {
    struct task_node_t {
      task_node_t* prev;
      uint32_t     task_space_id;
    };

    task_node_t* free_task_space_id;
    uint32_t     unused_task_space_start;
  } // namespace

  void init_task_space(memory::virtual_address_t root_page_table) {
    auto cap_table = capability::get_capability_table_by_id(1);
    while (cap_table->next_cap_space_id != 0) {
      cap_table = capability::get_capability_table_by_id(cap_table->next_cap_space_id);
    }

    memory::map(root_page_table,
                arch::memory::begin_of_task_space,
                memory::virt_to_phys(capability::get_capability(memory::pop_page()).memory.address()),
                memory::page_flag::readable | memory::page_flag::writable | memory::page_flag::executable | memory::page_flag::global);
    memory::map(root_page_table,
                arch::memory::begin_of_task_space + offsetof(task_t, kernel_stack),
                memory::virt_to_phys(capability::get_capability(memory::pop_page()).memory.address()),
                memory::page_flag::readable | memory::page_flag::writable | memory::page_flag::global);

    auto kernel_task                       = get_task_by_id(0);
    kernel_task->task_id                   = { .task_space_id = 0, .generation = 0, .system_id = 0 };
    kernel_task->front_capability_space_id = cap_table->this_cap_space_id;
    kernel_task->arch_task                 = {};
    kernel_task->arch_task.set_root_page_table(root_page_table);

    free_task_space_id      = nullptr;
    unused_task_space_start = 1;
  }

  task_id_t create_new_task() {
    if (free_task_space_id == nullptr) [[unlikely]] {
      if (unused_task_space_start == 0) [[unlikely]] {
        panic("unused_task_space_start == 0");
      }

      auto kernel_task          = get_kernel_task();
      auto task_root_page_table = capability::get_capability(memory::pop_page()).memory.address();
      memset(task_root_page_table.as<void*>(), 0, memory::page_size());

      memory::map(kernel_task->arch_task.get_root_page_table(),
                  arch::memory::begin_of_task_space + sizeof(task_t) * unused_task_space_start,
                  memory::virt_to_phys(capability::get_capability(memory::pop_page()).memory.address()),
                  memory::page_flag::readable | memory::page_flag::writable | memory::page_flag::executable | memory::page_flag::global);
      memory::map(kernel_task->arch_task.get_root_page_table(),
                  arch::memory::begin_of_task_space + sizeof(task_t) * unused_task_space_start + offsetof(task_t, kernel_stack),
                  memory::virt_to_phys(capability::get_capability(memory::pop_page()).memory.address()),
                  memory::page_flag::readable | memory::page_flag::writable | memory::page_flag::global);

      memory::shallow_copy_mapping(task_root_page_table,
                                   kernel_task->arch_task.get_root_page_table(),
                                   arch::memory::begin_of_kernel_mode_address_space,
                                   arch::memory::end_of_kernel_mode_address_space + 1);

      auto task                       = get_task_by_id(unused_task_space_start);
      task->arch_task                 = {};
      task->task_id                   = { .task_space_id = unused_task_space_start, .generation = 0, .system_id = 0 };
      task->front_capability_space_id = 0;

      task->arch_task.set_root_page_table(task_root_page_table);

      ++unused_task_space_start;

      return task->task_id;
    }

    auto task = get_task_by_id(free_task_space_id->task_space_id);
    task->task_id.generation++;
    task->front_capability_space_id = 0;

    return task->task_id;
  }
} // namespace caprese::task
