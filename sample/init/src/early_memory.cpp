#include <init/early_memory.h>
#include <lib/syscall.h>

namespace init {
  namespace {
    cap_handle_t init_task_cap;
    cap_handle_t early_memory_cap;
    uintptr_t    free_address;
    uintptr_t    alloc_base;
    size_t       alloc_offset;
  }; // namespace

  bool init_early_memory(boot_t* boot) {
    init_task_cap = boot->init_task_cap;
    free_address  = boot->free_address_start;

    early_memory_cap = fetch_mem_cap();
    if (early_memory_cap == 0) [[unlikely]] {
      return false;
    }

    auto [virt_addr, virt_addr_error] = sys_cap_get_field(early_memory_cap, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
    if (virt_addr_error) [[unlikely]] {
      return false;
    }

    alloc_base   = virt_addr;
    alloc_offset = 0;

    return true;
  }

  cap_handle_t fetch_mem_cap() {
    auto [page_size, page_size_error] = sys_base_page_size();
    if (page_size_error) [[unlikely]] {
      return 0;
    }

    auto [list_size, list_size_error] = sys_cap_list_size();
    if (list_size_error) [[unlikely]] {
      return 0;
    }

    for (cap_handle_t handle = 0; handle < list_size; ++handle) {
      if (get_cap_type(handle) != CAP_TYPE_MEMORY) [[unlikely]] {
        continue;
      }

      auto [virt_addr, virt_addr_error] = sys_cap_get_field(handle, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
      if (virt_addr || virt_addr_error) [[unlikely]] {
        continue;
      }

      sysret_t result = sys_cap_call_method(handle, MEMORY_CAP_METHOD_MAP, init_task_cap, free_address, MEMORY_CAP_CONSTANT_READABLE | MEMORY_CAP_CONSTANT_WRITABLE);
      if (result.error) [[unlikely]] {
        return 0;
      }

      free_address += page_size;

      return handle;
    }

    return 0;
  }

  void* early_alloc(size_t size) {
    auto [page_size, page_size_error] = sys_base_page_size();
    if (page_size_error) [[unlikely]] {
      return nullptr;
    }

    void* result = reinterpret_cast<void*>(alloc_base + alloc_offset);
    alloc_offset += size;
    if (alloc_offset > page_size) {
      return nullptr;
    }

    return result;
  }

  void early_free_all() {
    sys_cap_call_method(early_memory_cap, MEMORY_CAP_METHOD_UNMAP);
  }
} // namespace init
