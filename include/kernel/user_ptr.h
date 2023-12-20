#ifndef KERNEL_USER_H_
#define KERNEL_USER_H_

#include <kernel/address.h>
#include <kernel/page.h>
#include <kernel/task.h>

template<typename T>
struct user_ptr {
  uintptr_t       _ptr;
  map_ptr<task_t> _task;

  bool copy_to(map_ptr<T> dst) {
    size_t written = 0;

    while (written < sizeof(T)) {
      map_ptr<page_table_t> page_table = _task->root_page_table;
      map_ptr<pte_t>        pte        = 0_map;
      int                   level      = MAX_PAGE_TABLE_LEVEL;
      uintptr_t             va         = _ptr + written;

      for (; level >= 0; --level) {
        pte = page_table->walk(make_virt_ptr(va), level);
        if (pte->is_disabled()) {
          return false;
        }

        if (!pte->is_table()) {
          break;
        }

        page_table = pte->get_next_page().as<page_table_t>();
      }

      if (pte->is_disabled() || pte->is_table() || !pte->is_user()) {
        return false;
      }

      size_t page_size = get_page_size(level);
      size_t offset    = va & (page_size - 1);
      size_t size      = page_size - offset;
      if (size > sizeof(T) - written) {
        size = sizeof(T) - written;
      }

      memcpy((dst.template as<void>() + written).get(), (pte->get_next_page() + offset).get(), size);
      written += size;
    }

    return true;
  }

  bool copy_from(map_ptr<T> src) {
    size_t written = 0;

    while (written < sizeof(T)) {
      map_ptr<page_table_t> page_table = _task->root_page_table;
      map_ptr<pte_t>        pte        = 0_map;
      int                   level      = MAX_PAGE_TABLE_LEVEL;
      uintptr_t             va         = _ptr + written;

      for (; level >= 0; --level) {
        pte = page_table->walk(make_virt_ptr(va), level);
        if (pte->is_disabled()) {
          return false;
        }

        if (!pte->is_table()) {
          break;
        }

        page_table = pte->get_next_page().as<page_table_t>();
      }

      if (pte->is_disabled() || pte->is_table() || !pte->is_user()) {
        return false;
      }

      size_t page_size = get_page_size(level);
      size_t offset    = va & (page_size - 1);
      size_t size      = page_size - offset;
      if (size > sizeof(T) - written) {
        size = sizeof(T) - written;
      }

      memcpy((pte->get_next_page() + offset).get(), (src + written).get(), size);
      written += size;
    }

    return true;
  }

  constexpr static user_ptr from(map_ptr<task_t> task, uintptr_t ptr) {
    return user_ptr { ptr, task };
  }

  constexpr static user_ptr from(map_ptr<task_t> task, const void* ptr) {
    return user_ptr { reinterpret_cast<uintptr_t>(ptr), task };
  }
};

#endif // KERNEL_USER_H_
