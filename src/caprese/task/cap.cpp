#include <caprese/memory/page.h>
#include <caprese/task/cap.h>
#include <caprese/task/task.h>
#include <caprese/util/panic.h>

namespace caprese::task {
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

    if (cap->info.tid != std::bit_cast<uint32_t>(null_tid)) [[unlikely]] {
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

  cid_handle_t move_capability(task_t* dst_task, task_t* src_task, cid_handle_t handle) {
    cid_t* cid = lookup_cid(src_task, handle);
    if (cid == nullptr) [[unlikely]] {
      return 0;
    }

    capability::capability_t* cap = lookup_capability(src_task, *cid);
    if (cap == nullptr) [[unlikely]] {
      return 0;
    }

    capability::class_t* cap_class = capability::lookup_class(cap->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return 0;
    }
    if ((cap_class->flags & capability::CLASS_FLAG_MOVABLE) == 0) [[unlikely]] {
      return 0;
    }

    cap->info.tid           = std::bit_cast<uint32_t>(null_tid);
    cid_handle_t dst_handle = insert_capability(dst_task, cap);
    if (dst_handle == 0) [[unlikely]] {
      cap->info.tid = std::bit_cast<uint32_t>(src_task->tid);
      return 0;
    }

    cid->ccid               = 0;
    cid->generation         = 0;
    cid->prev_free_cap_list = src_task->free_cap_list;

    src_task->free_cap_list = handle;

    return dst_handle;
  }

  size_t allocated_cap_list_size(task_t* task) {
    return task->used_cap_space_count * (arch::PAGE_SIZE / sizeof(cid_t));
  }
} // namespace caprese::task
