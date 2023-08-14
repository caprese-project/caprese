#include <bit>
#include <cstdlib>

#include <caprese/arch/rv64/builtin_capability.h>
#include <caprese/capability/capability.h>
#include <caprese/memory/page.h>
#include <caprese/task/task.h>

namespace caprese::arch::inline rv64 {
  namespace {
    using namespace caprese::capability;

    capret_t memory_cap_method_move(
        capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
      (void)cap;
      // TODO: impl
      return { .result = 0, .error = 0 };
    }

    capret_t memory_cap_method_copy(
        capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
      (void)cap;
      // TODO: impl
      return { .result = 0, .error = 0 };
    }

    capret_t
        memory_cap_method_map(capability_t* cap, uintptr_t _new_task_cap_cid, uintptr_t _virtual_address, uintptr_t _flags, [[maybe_unused]] uintptr_t arg3) {
#if CONFIG_USER_SPACE_BASE > 0
      if (_virtual_address < CONFIG_USER_SPACE_BASE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
#endif // CONFIG_USER_SPACE_BASE > 0

      if (_virtual_address >= CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      task::task_t* current_task = task::get_current_task();

      cid_t         new_task_cap_cid = std::bit_cast<cid_t>(static_cast<uint32_t>(_new_task_cap_cid));
      capability_t* new_task_cap     = task::lookup_capability(current_task, new_task_cap_cid);

      if (new_task_cap == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (new_task_cap->ccid != TASK_CAP_CCID) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capret_t new_tid_field = get_field(new_task_cap, TASK_CAP_FIELD_TID);
      if (new_tid_field.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      task::tid_t new_tid = std::bit_cast<task::tid_t>(static_cast<uint32_t>(new_tid_field.result));

      task::task_t* new_task = task::lookup(new_tid);
      if (new_task == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      memory::mapped_address_t new_task_root_page_table = task::get_root_page_table(new_task);

      capret_t physical_address_field = get_field(cap, MEMORY_CAP_FIELD_PHYSICAL_ADDRESS);
      if (physical_address_field.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      arch::page_flags_t flags {
        .readable   = (static_cast<uint8_t>(_flags) & MEMORY_CAP_MAP_FLAG_READABLE) >> std::countr_zero(MEMORY_CAP_MAP_FLAG_READABLE),
        .writable   = (static_cast<uint8_t>(_flags) & MEMORY_CAP_MAP_FLAG_WRITABLE) >> std::countr_zero(MEMORY_CAP_MAP_FLAG_WRITABLE),
        .executable = (static_cast<uint8_t>(_flags) & MEMORY_CAP_MAP_FLAG_EXECUTABLE) >> std::countr_zero(MEMORY_CAP_MAP_FLAG_EXECUTABLE),
        .user       = 1,
      };

      capret_t readable_permission = is_permitted(cap, MEMORY_CAP_PERMISSION_READABLE);
      if (readable_permission.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      capret_t writable_permission = is_permitted(cap, MEMORY_CAP_PERMISSION_WRITABLE);
      if (writable_permission.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      capret_t executable_permission = is_permitted(cap, MEMORY_CAP_PERMISSION_EXECUTABLE);
      if (executable_permission.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      if (flags.readable && !readable_permission.result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (flags.writable && !writable_permission.result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (flags.executable && !executable_permission.result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capret_t old_tid_field = get_field(cap, MEMORY_CAP_FIELD_TID);
      if (old_tid_field.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      task::tid_t old_tid = std::bit_cast<task::tid_t>(static_cast<uint32_t>(old_tid_field.result));

      task::task_t* old_task = task::lookup(old_tid);
      if (old_task != nullptr) {
        capret_t old_virtual_address_field = get_field(cap, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
        if (old_virtual_address_field.error) [[unlikely]] {
          return { .result = 0, .error = 1 };
        }
        memory::mapped_address_t  old_task_root_page_table = task::get_root_page_table(old_task);
        memory::virtual_address_t old_virtual_address      = memory::virtual_address_t::from(old_virtual_address_field.result);
        if (memory::is_mapped(old_task_root_page_table, old_virtual_address)) {
          bool result = memory::unmap(old_task_root_page_table, old_virtual_address);
          if (!result) [[unlikely]] {
            return { .result = 0, .error = 1 };
          }
        }
      }

      memory::virtual_address_t  virtual_address  = memory::virtual_address_t::from(_virtual_address);
      memory::physical_address_t physical_address = memory::physical_address_t::from(physical_address_field.result);

      bool result = memory::map(new_task_root_page_table, virtual_address, physical_address, flags, true);
      if (!result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      set_field(cap, MEMORY_CAP_FIELD_TID, std::bit_cast<uint32_t>(new_tid));
      set_field(cap, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS, _virtual_address);

      return { .result = 0, .error = 0 };
    }

    capret_t memory_cap_method_unmap(
        capability_t* cap, uintptr_t _task_cap_cid, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
      task::task_t* current_task = task::get_current_task();

      cid_t         task_cap_cid = std::bit_cast<cid_t>(static_cast<uint32_t>(_task_cap_cid));
      capability_t* task_cap     = task::lookup_capability(current_task, task_cap_cid);

      if (task_cap == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (task_cap->ccid != TASK_CAP_CCID) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capret_t tid_field = get_field(task_cap, TASK_CAP_FIELD_TID);
      if (tid_field.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      task::tid_t tid = std::bit_cast<task::tid_t>(static_cast<uint32_t>(tid_field.result));

      task::task_t* task = task::lookup(tid);
      if (task == nullptr) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      memory::mapped_address_t task_root_page_table = task::get_root_page_table(task);

      capret_t virtual_address_field = get_field(cap, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
      if (virtual_address_field.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      memory::virtual_address_t virtual_address = memory::virtual_address_t::from(virtual_address_field.result);

      if (!memory::is_mapped(task_root_page_table, virtual_address)) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      bool result = memory::unmap(task_root_page_table, virtual_address);
      if (!result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      set_field(cap, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS, 0);
      set_field(cap, MEMORY_CAP_FIELD_TID, 0);

      return { .result = 0, .error = 0 };
    }

    capret_t memory_cap_method_read(capability_t* cap, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
      capret_t permission = is_permitted(cap, MEMORY_CAP_PERMISSION_READABLE);
      if (permission.error || !permission.result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (offset >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (size >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (offset + size >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (size > 8) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capret_t phys_addr = get_field(cap, MEMORY_CAP_FIELD_PHYSICAL_ADDRESS);
      if (phys_addr.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      memory::mapped_address_t page   = memory::physical_address_t::from(phys_addr.result).mapped_address();
      uint64_t                 result = 0;
      for (size_t i = 0; i < size; ++i) {
        result |= page.as<uint8_t>()[offset + i] << (8 * i);
      }

      return { .result = result, .error = 0 };
    }

    capret_t memory_cap_method_write(capability_t* cap, uintptr_t value, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg3) {
      capret_t permission = is_permitted(cap, MEMORY_CAP_PERMISSION_WRITABLE);
      if (permission.error || !permission.result) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (offset >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (size >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (offset + size >= PAGE_SIZE) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      if (size > 8) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }

      capret_t phys_addr = get_field(cap, MEMORY_CAP_FIELD_PHYSICAL_ADDRESS);
      if (phys_addr.error) [[unlikely]] {
        return { .result = 0, .error = 1 };
      }
      memory::mapped_address_t page = memory::physical_address_t::from(phys_addr.result).mapped_address();
      for (size_t i = 0; i < size; ++i) {
        page.as<uint8_t>()[offset + i] = (value >> (8 * i)) & 0xff;
      }

      return { .result = 0, .error = 0 };
    }

    void create_memory_cap_class() {
      class_t* cap_class = reinterpret_cast<class_t*>(CONFIG_CAPABILITY_CLASS_SPACE_BASE);

      cap_class->name            = "memory";
      cap_class->flags           = CLASS_FLAG_VALID | CLASS_FLAG_BUILTIN | CLASS_FLAG_MOVABLE | CLASS_FLAG_COPYABLE;
      cap_class->num_permissions = 4;
      cap_class->num_fields      = 3;
      cap_class->num_methods     = 6;
      cap_class->idx_move        = 0;
      cap_class->idx_copy        = 1;
      cap_class->methods         = static_cast<method_t*>(malloc(cap_class->num_methods * sizeof(method_t)));

      cap_class->methods[MEMORY_CAP_METHOD_MOVE]  = memory_cap_method_move;
      cap_class->methods[MEMORY_CAP_METHOD_COPY]  = memory_cap_method_copy;
      cap_class->methods[MEMORY_CAP_METHOD_MAP]   = memory_cap_method_map;
      cap_class->methods[MEMORY_CAP_METHOD_UNMAP] = memory_cap_method_unmap;
      cap_class->methods[MEMORY_CAP_METHOD_READ]  = memory_cap_method_read;
      cap_class->methods[MEMORY_CAP_METHOD_WRITE] = memory_cap_method_write;
    }

    void create_task_cap_class() { }

    void create_trap_cap_class() { }
  } // namespace

  void create_builtin_capability_classes() {
    memory::mapped_address_t root_page_table = memory::get_current_root_page_table();
    memory::mapped_address_t page            = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));

    memory::map(root_page_table,
                memory::virtual_address_t::from(CONFIG_CAPABILITY_CLASS_SPACE_BASE),
                page.physical_address(),
                { .readable = true, .writable = true, .executable = false, .user = false },
                true);

    create_memory_cap_class();
    create_task_cap_class();
    create_trap_cap_class();
  }
} // namespace caprese::arch::inline rv64
