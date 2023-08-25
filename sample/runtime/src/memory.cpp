#include <algorithm>

#include <caprese/capability/bic/memory.h>

#include <runtime/cap.h>
#include <runtime/global.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>

namespace {
  uintptr_t current_heap_size;
} // namespace

extern "C" {
  void __init_heap() {
    current_heap_size = 0;
  }

  uintptr_t get_physical_address(memory_handle_t handle) {
    sysret_t sysret = sys_cap_get_field(handle, caprese::capability::bic::memory::field::PHYSICAL_ADDRESS);
    if (sysret.error) [[unlikely]] {
      return 0;
    }
    return sysret.result;
  }

  uintptr_t get_virtual_address(memory_handle_t handle) {
    sysret_t sysret = sys_cap_get_field(handle, caprese::capability::bic::memory::field::VIRTUAL_ADDRESS);
    if (sysret.error) [[unlikely]] {
      return 0;
    }
    return sysret.result;
  }

  tid_t get_mapped_tid(memory_handle_t handle) {
    sysret_t sysret = sys_cap_get_field(handle, caprese::capability::bic::memory::field::TID);
    if (sysret.error) [[unlikely]] {
      return 0;
    }
    return sysret.result;
  }

  bool is_mapped(memory_handle_t handle) {
    return get_virtual_address(handle) != 0;
  }

  bool map_memory_page_to_task(task_handle_t task_handle, memory_handle_t memory_handle, uintptr_t virtual_address, unsigned _flags) {
    uintptr_t flags = 0;
    if (_flags & MEMORY_FLAG_READABLE) {
      flags |= caprese::capability::bic::memory::constant::READABLE;
    }
    if (_flags & MEMORY_FLAG_WRITABLE) {
      flags |= caprese::capability::bic::memory::constant::WRITABLE;
    }
    if (_flags & MEMORY_FLAG_EXECUTABLE) {
      flags |= caprese::capability::bic::memory::constant::EXECUTABLE;
    }
    sysret_t sysret = sys_cap_call_method(memory_handle, caprese::capability::bic::memory::method::MAP, task_handle, virtual_address, flags, 0);
    return sysret.error == 0;
  }

  bool map_memory_page(memory_handle_t handle, uintptr_t virtual_address, unsigned flags) {
    return map_memory_page_to_task(__this_task_handle, handle, virtual_address, flags);
  }

  bool unmap_memory_page(memory_handle_t handle) {
    sysret_t sysret = sys_cap_call_method(handle, caprese::capability::bic::memory::method::UNMAP, 0, 0, 0, 0);
    return sysret.error == 0;
  }

  bool read_memory(memory_handle_t handle, void* dst, size_t offset, size_t size) {
    if (offset + size > __page_size) {
      return false;
    }

    sysret_t sysret;
    sysret = sys_cap_is_permitted(handle, caprese::capability::bic::memory::permission::READABLE);
    if (sysret.error || sysret.result == 0) [[unlikely]] {
      return false;
    }

    for (size_t i = 0; i < size; i += sizeof(uintptr_t)) {
      size_t sz = std::min(size - i, sizeof(uintptr_t));
      sysret    = sys_cap_call_method(handle, caprese::capability::bic::memory::method::READ, offset + i, sz, 0, 0);
      if (sysret.error) [[unlikely]] {
        return false;
      }

      for (size_t j = 0; j < sz; ++j) {
        static_cast<char*>(dst)[i + j] = reinterpret_cast<const char*>(&sysret.result)[j];
      }
    }

    return true;
  }

  bool write_memory(memory_handle_t handle, const void* src, size_t offset, size_t size) {
    if (offset + size > __page_size) {
      return false;
    }

    sysret_t sysret;
    sysret = sys_cap_is_permitted(handle, caprese::capability::bic::memory::permission::WRITABLE);
    if (sysret.error || sysret.result == 0) [[unlikely]] {
      return false;
    }

    for (size_t i = 0; i < size; i += sizeof(uintptr_t)) {
      size_t sz = std::min(size - i, sizeof(uintptr_t));
      sysret    = sys_cap_call_method(handle, caprese::capability::bic::memory::method::WRITE, *reinterpret_cast<const uintptr_t*>(static_cast<const char*>(src) + i), offset + i, sz, 0);
      if (sysret.error) [[unlikely]] {
        return false;
      }
    }

    return true;
  }

  memory_handle_t fetch_memory_handle(unsigned flags) {
    sysret_t sysret = sys_cap_list_size();
    if (sysret.error == 0) {
      for (handle_t handle = 0; handle < sysret.result; ++handle) {
        if (get_handle_type(handle) != HANDLE_TYPE_MEMORY) {
          continue;
        }

        if (!is_mapped(handle)) {
          if (map_memory_page_to_heap(handle, flags) != nullptr) {
            return handle;
          }
        }
      }
    }

    // TODO: impl; ipc(mm task)
    return 0;
  }

  void* map_memory_page_to_heap(memory_handle_t handle, unsigned flags) {
    if (map_memory_page(handle, __heap_start + current_heap_size, flags)) {
      current_heap_size += __page_size;
      return reinterpret_cast<void*>(__heap_start + current_heap_size);
    }
    return nullptr;
  }
}
