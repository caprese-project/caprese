#include <cstdlib>
#include <cstring>

#include <caprese/capability/capability.h>
#include <caprese/memory/page.h>
#include <caprese/task/task.h>
#include <caprese/util/align.h>
#include <caprese/util/panic.h>

namespace caprese::capability {
  namespace {
    memory::virtual_address_t next_capability_class_space_address;
    size_t                    next_capability_class_space_offset;
    capability_t*             free_capability_list;
    size_t                    capability_block_position;
    uint32_t                  cid_generation_counter;

    [[nodiscard]] bool init_capability_block(capability_t* cap_block) {
      for (size_t offset = 0; offset < arch::PAGE_SIZE / sizeof(capability_t); ++offset) {
        cap_block[offset].info.ccid                  = 0;
        cap_block[offset].info.cid_generation        = cid_generation_counter;
        cap_block[offset].management.prev_free_list  = 0;
        cap_block[offset].management.next_free_list  = 0;
        cap_block[offset].management.prev_free_index = (offset + 1) % (arch::PAGE_SIZE / sizeof(capability_t));
      }
      cid_generation_counter = (cid_generation_counter + 1) % (1 << (8 * sizeof(cap_gen_t)));

      return true;
    }

    [[nodiscard]] bool allocate_new_capability_block() {
      memory::mapped_address_t page = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
      if (page.is_null()) [[unlikely]] {
        return false;
      }

      memory::mapped_address_t root_page_table = task::get_kernel_root_page_table();

      size_t position = capability_block_position;
      do {
        memory::virtual_address_t block_page = memory::virtual_address_t::from(CONFIG_CAPABILITY_SPACE_BASE + position);
        if (!memory::is_mapped(root_page_table, block_page)) {
          bool result = memory::map(root_page_table, block_page, page.physical_address(), { .readable = true, .writable = true, .executable = false, .user = false, .global = true }, true);
          if (!result) [[unlikely]] {
            return false;
          }

          capability_t* new_capability_block = block_page.as<capability_t>();
          if (!init_capability_block(new_capability_block)) [[unlikely]] {
            return false;
          }

          if (free_capability_list != nullptr) {
            new_capability_block->management.prev_free_list = make_ref(free_capability_list);
            free_capability_list->management.next_free_list = make_ref(new_capability_block);
          } else {
            new_capability_block->management.prev_free_list = 0;
          }
          free_capability_list = new_capability_block;

          capability_block_position = position;
          return true;
        }
        position = (position + arch::PAGE_SIZE) % CONFIG_CAPABILITY_SPACE_SIZE;
      } while (position != capability_block_position);

      return false;
    }
  } // namespace

  bool init_capability_class_space() {
    next_capability_class_space_address = memory::virtual_address_t::from(CONFIG_CAPABILITY_CLASS_SPACE_BASE);
    next_capability_class_space_offset  = 1;
    return true;
  }

  bool init_capability_space() {
    free_capability_list      = nullptr;
    capability_block_position = 0;
    cid_generation_counter    = 0;
    return allocate_new_capability_block();
  }

  class_t* create_capability_class() {
    if (next_capability_class_space_address.value - CONFIG_CAPABILITY_CLASS_SPACE_BASE >= CONFIG_CAPABILITY_CLASS_SPACE_SIZE) [[unlikely]] {
      return nullptr;
    }

    class_t*                 cap_class       = next_capability_class_space_address.as<class_t>() + next_capability_class_space_offset;
    memory::mapped_address_t root_page_table = memory::get_current_root_page_table();

    if (!memory::is_mapped(root_page_table, next_capability_class_space_address)) [[unlikely]] {
      memory::mapped_address_t page = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
      if (page.is_null()) [[unlikely]] {
        return nullptr;
      }

      bool result = memory::map(root_page_table,
                                next_capability_class_space_address,
                                page.physical_address(),
                                { .readable = true, .writable = true, .executable = false, .user = false, .global = true },
                                true);
      if (!result) [[unlikely]] {
        return nullptr;
      }
    }

    ++next_capability_class_space_offset;
    if (sizeof(class_t) * next_capability_class_space_offset >= arch::PAGE_SIZE) [[unlikely]] {
      next_capability_class_space_address.value += arch::PAGE_SIZE;
      next_capability_class_space_offset = 0;
    }

    return cap_class;
  }

  size_t instance_size(class_t* cap_class) {
    size_t permissions_size = round_up(round_up(cap_class->num_permissions, 8) / 8, sizeof(uintptr_t));
    size_t fields_size      = cap_class->num_fields * sizeof(uintptr_t);
    return permissions_size + fields_size;
  }

  capability_t* create_capability(ccid_t ccid) {
    class_t* cap_class = lookup_class(ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return nullptr;
    }

    void* instance = malloc(instance_size(cap_class));
    if (instance == nullptr) [[unlikely]] {
      return nullptr;
    }

    if (free_capability_list == nullptr) [[unlikely]] {
      if (!allocate_new_capability_block()) [[unlikely]] {
        return nullptr;
      }
    }

    capability_t* capability_block = reinterpret_cast<capability_t*>(reinterpret_cast<uintptr_t>(free_capability_list) & ~(arch::PAGE_SIZE - 1));

    capability_t* result = capability_block + free_capability_list->management.prev_free_index;
    if (result == free_capability_list) [[unlikely]] {
      if (free_capability_list->management.prev_free_list == 0) [[unlikely]] {
        free_capability_list = nullptr;
      } else {
        free_capability_list = reinterpret_cast<capability_t*>(CONFIG_CAPABILITY_SPACE_BASE) + free_capability_list->management.prev_free_list;
      }
    } else {
      free_capability_list->management.prev_free_index = result->management.prev_free_index;
    }

    result->info.ccid = ccid;
    ++result->info.cid_generation;
    result->info.tid      = 0;
    result->info.instance = memory::mapped_address_t::from(instance);

    return result;
  }

  void delete_capability(capability_t* capability) {
    if (capability->ccid == 0) [[unlikely]] {
      return;
    }

    capability_t* capability_block = reinterpret_cast<capability_t*>(reinterpret_cast<uintptr_t>(capability) & ~(arch::PAGE_SIZE - 1));
    size_t        index            = (reinterpret_cast<uintptr_t>(capability) & (arch::PAGE_SIZE - 1)) / sizeof(capability_t);

    size_t count = 0;
    for (size_t offset = 0; offset < arch::PAGE_SIZE / sizeof(capability_t); ++offset) {
      if (capability_block[offset].ccid == 0) {
        if (count == 0) [[unlikely]] {
          capability->management.prev_free_index              = capability_block[offset].management.prev_free_index;
          capability_block[offset].management.prev_free_index = index;
        }
        ++count;
      }
    }

    capability->info.ccid = 0;
    free(capability->info.instance.as<void>());

    capability->management.next_free_list = 0;
    capability->management.prev_free_list = 0;

    if (count == arch::PAGE_SIZE / sizeof(capability_t) - 1) [[unlikely]] {
      free(capability_block);
    } else if (count == 0) [[unlikely]] {
      if (free_capability_list != nullptr) {
        capability->management.prev_free_list           = make_ref(free_capability_list);
        free_capability_list->management.next_free_list = make_ref(capability);
      } else {
        capability->management.prev_free_list = 0;
      }
      free_capability_list = capability;
    }
  }

  [[nodiscard]] capability_t* copy_capability(capability_t* capability, uint64_t permissions) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return nullptr;
    }

    if ((cap_class->flags & CLASS_FLAG_COPYABLE) == 0) [[unlikely]] {
      return nullptr;
    }

    capability_t* cap = create_capability(capability->ccid);
    if (cap == nullptr) [[unlikely]] {
      return nullptr;
    }

    memcpy(cap->info.instance.as<void>(), capability->info.instance.as<void>(), instance_size(cap_class));
    for (size_t i = 0; i < cap_class->num_permissions; ++i) {
      if (permissions & (1 << i)) {
        continue;
      }
      set_permission(cap, i, false);
    }

    return cap;
  }

  class_t* lookup_class(ccid_t ccid) {
    if (ccid == 0) [[unlikely]] {
      return nullptr;
    }

    class_t*  cap_class = reinterpret_cast<class_t*>(CONFIG_CAPABILITY_CLASS_SPACE_BASE) + ccid;
    uintptr_t page      = reinterpret_cast<uintptr_t>(cap_class) & ~(arch::PAGE_SIZE - 1);

    memory::mapped_address_t root_page_table = task::get_kernel_root_page_table();
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }

    return cap_class;
  }

  ccid_t make_ccid(class_t* cap_class) {
    return static_cast<ccid_t>((reinterpret_cast<uintptr_t>(cap_class) - CONFIG_CAPABILITY_CLASS_SPACE_BASE) / CONFIG_CAPABILITY_CLASS_SIZE);
  }

  cap_ref_t make_ref(capability_t* capability) {
    return static_cast<cap_ref_t>((reinterpret_cast<uintptr_t>(capability) - CONFIG_CAPABILITY_SPACE_BASE) >> CONFIG_CAPABILITY_SIZE_BIT);
  }

  cap_ret_t call_method(capability_t* capability, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (method >= cap_class->num_methods) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    return cap_class->methods[method](capability, arg0, arg1, arg2, arg3);
  }

  void set_field(capability_t* capability, uint8_t field, uintptr_t value) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return;
    }
    if (field >= cap_class->num_fields) [[unlikely]] {
      return;
    }

    uintptr_t offset  = round_up(round_up(cap_class->num_permissions, 8) / 8, sizeof(uintptr_t));
    uintptr_t base    = capability->info.instance.value + offset;
    uintptr_t address = base + field * sizeof(uintptr_t);

    *reinterpret_cast<uintptr_t*>(address) = value;
  }

  cap_ret_t get_field(capability_t* capability, uint8_t field) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (field >= cap_class->num_fields) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    uintptr_t offset  = round_up(round_up(cap_class->num_permissions, 8) / 8, sizeof(uintptr_t));
    uintptr_t base    = capability->info.instance.value + offset;
    uintptr_t address = base + field * sizeof(uintptr_t);
    uintptr_t value   = *reinterpret_cast<uintptr_t*>(address);

    return { .result = value, .error = 0 };
  }

  void set_permission(capability_t* capability, uint8_t permission, bool value) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return;
    }
    if (permission >= cap_class->num_permissions) [[unlikely]] {
      return;
    }

    uintptr_t base = capability->info.instance.value + permission / 8;
    *reinterpret_cast<uint8_t*>(base) &= ~(1 << (permission % 8));
    *reinterpret_cast<uint8_t*>(base) |= (static_cast<uint8_t>(value) << (permission % 8));
  }

  cap_ret_t is_permitted(capability_t* capability, uint8_t permission) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (permission >= cap_class->num_permissions) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    uintptr_t base  = capability->info.instance.value + permission / 8;
    uintptr_t value = (*reinterpret_cast<uint8_t*>(base)) & (1 << (permission % 8));

    return { .result = (uintptr_t)(value != 0), .error = 0 };
  }
} // namespace caprese::capability
