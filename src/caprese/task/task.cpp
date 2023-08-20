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
#include <cstring>

#include <caprese/memory/cls.h>
#include <caprese/memory/kernel_space.h>
#include <caprese/memory/page.h>
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
    // TODO: impl
    (void)task;
  }

  void switch_to(task_t* task) {
    if (task->state != TASK_STATE_READY && task->state != TASK_STATE_CREATING) [[unlikely]] {
      panic("Attempted to switch to a non-ready task.");
    }
    task_t* current_task           = get_current_task();
    current_task->state            = TASK_STATE_READY;
    task->state                    = TASK_STATE_RUNNING;
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

  cid_t make_cid(capability::capability_t* cap) {
    if (cap == nullptr || cap->ccid == 0) [[unlikely]] {
      panic("Attempt to make a cid from null cap.");
    }
    cid_t cid;
    cid.ccid       = cap->ccid;
    cid.generation = cap->info.cid_generation;
    cid.cap_ref    = capability::make_ref(cap);
    return cid;
  }

  cid_t* lookup_cid(task_t* task, cid_handle_t handle) {
    memory::mapped_address_t  root_page_table = task::get_root_page_table(task);
    memory::virtual_address_t cid_page        = memory::virtual_address_t::from(CONFIG_CAPABILITY_LIST_SPACE_BASE + arch::PAGE_SIZE * (handle / (arch::PAGE_SIZE / sizeof(cid_t))));

    memory::mapped_address_t base = memory::get_mapped_address(root_page_table, cid_page);
    if (base.is_null()) [[unlikely]] {
      return nullptr;
    }
    return base.as<cid_t>() + handle % (arch::PAGE_SIZE / sizeof(cid_t));
  }

  capability::capability_t* lookup_capability(task_t* task, cid_t cid) {
    if (cid.ccid == 0) [[unlikely]] {
      return nullptr;
    }

    capability::capability_t* cap  = reinterpret_cast<capability::capability_t*>(CONFIG_CAPABILITY_SPACE_BASE) + cid.cap_ref;
    uintptr_t                 page = reinterpret_cast<uintptr_t>(cap) & ~(arch::PAGE_SIZE - 1);

    memory::mapped_address_t root_page_table = get_root_page_table(task);
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }

    if (cap->info.ccid != cid.ccid) [[unlikely]] {
      return nullptr;
    }
    if (cap->info.cid_generation != cid.generation) [[unlikely]] {
      return nullptr;
    }
    if (cap->info.tid != std::bit_cast<uint32_t>(task->tid)) [[unlikely]] {
      return nullptr;
    }

    return cap;
  }

  cid_handle_t insert_capability(task_t* task, capability::capability_t* cap) {
    if (cap->ccid == 0) [[unlikely]] {
      panic("Attempted to insert null capability.");
    }

    if (cap->info.tid != 0) [[unlikely]] {
      panic("Attempted to insert capability in use.");
    }

    if (task->free_cap_list == 0) [[unlikely]] {
      memory::mapped_address_t  page      = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
      memory::virtual_address_t cap_space = memory::virtual_address_t::from(CONFIG_CAPABILITY_LIST_SPACE_BASE + arch::PAGE_SIZE * task->used_cap_space_count);

      memory::mapped_address_t root_page_table = task::get_root_page_table(task);
      bool result = memory::map(root_page_table, cap_space, page.physical_address(), { .readable = true, .writable = false, .executable = false, .user = true, .global = false }, true);
      if (!result) [[unlikely]] {
        return 0;
      }

      // If this cap is used in aligned_alloc or map, this cap is invalid
      if (cap->ccid == 0) [[unlikely]] {
        return 0;
      }

      cid_t*       base        = page.as<cid_t>();
      cid_handle_t base_handle = arch::PAGE_SIZE / sizeof(cid_t) * task->used_cap_space_count;
      for (cid_handle_t offset = 0; offset < arch::PAGE_SIZE / sizeof(cid_t); ++offset) {
        cid_t*       cid        = base + offset;
        cid_handle_t handle     = base_handle + offset;
        cid->ccid               = 0;
        cid->generation         = 0;
        cid->prev_free_cap_list = task->free_cap_list;
        task->free_cap_list     = handle;
      }
      ++task->used_cap_space_count;
    }

    cid_handle_t handle = task->free_cap_list;
    cid_t*       cid    = lookup_cid(task, handle);
    if (cid == nullptr) [[unlikely]] {
      return 0;
    }

    task->free_cap_list = cid->prev_free_cap_list;
    *cid                = make_cid(cap);
    cap->info.tid       = std::bit_cast<uint32_t>(task->tid);

    return handle;
  }

  cid_t move_capability(task_t* dst_task, task_t* src_task, cid_handle_t handle) {
    cid_t* cid = lookup_cid(src_task, handle);
    if (cid == nullptr) [[unlikely]] {
      return null_cid();
    }

    capability::capability_t* cap = lookup_capability(src_task, *cid);
    if (cap == nullptr) [[unlikely]] {
      return null_cid();
    }

    // TODO: check movablity

    cap->info.tid           = 0;
    cid_handle_t dst_handle = insert_capability(dst_task, cap);
    if (dst_handle == 0) [[unlikely]] {
      cap->info.tid = std::bit_cast<uint32_t>(src_task->tid);
      return null_cid();
    }

    cid->ccid               = 0;
    cid->generation         = 0;
    cid->prev_free_cap_list = src_task->free_cap_list;

    src_task->free_cap_list = handle;

    return *lookup_cid(dst_task, dst_handle);
  }

  size_t allocated_cap_list_size(task_t* task) {
    return task->used_cap_space_count * (arch::PAGE_SIZE / sizeof(cid_t));
  }
} // namespace caprese::task
