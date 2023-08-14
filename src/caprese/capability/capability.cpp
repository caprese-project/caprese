#include <caprese/capability/capability.h>
#include <caprese/memory/page.h>
#include <caprese/task/task.h>
#include <caprese/util/align.h>

namespace caprese::capability {
  class_t* lookup_class(ccid_t ccid) {
    class_t*  cap_class = reinterpret_cast<class_t*>(CONFIG_CAPABILITY_CLASS_SPACE_BASE) + ccid;
    uintptr_t page      = reinterpret_cast<uintptr_t>(cap_class) & ~(arch::PAGE_SIZE - 1);

    memory::mapped_address_t root_page_table = task::get_kernel_root_page_table();
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }

    return cap_class;
  }

  capability_t* lookup(cid_t cid) {
    capability_t* cap  = reinterpret_cast<capability_t*>(CONFIG_CAPABILITY_SPACE_BASE) + cid.index;
    uintptr_t     page = reinterpret_cast<uintptr_t>(cap) & ~(arch::PAGE_SIZE - 1);

    memory::mapped_address_t root_page_table = task::get_kernel_root_page_table();
    if (!memory::is_mapped(root_page_table, memory::virtual_address_t::from(page))) [[unlikely]] {
      return nullptr;
    }

    if (cap->cid_generation != cid.generation) [[unlikely]] {
      return nullptr;
    }

    return cap;
  }

  capret_t call_method(capability_t* capability, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
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
    uintptr_t base    = capability->instance.value + offset;
    uintptr_t address = base + field * sizeof(uintptr_t);

    *reinterpret_cast<uintptr_t*>(address) = value;
  }

  capret_t get_field(capability_t* capability, uint8_t field) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (field >= cap_class->num_fields) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    uintptr_t offset  = round_up(round_up(cap_class->num_permissions, 8) / 8, sizeof(uintptr_t));
    uintptr_t base    = capability->instance.value + offset;
    uintptr_t address = base + field * sizeof(uintptr_t);
    uintptr_t value   = *reinterpret_cast<uintptr_t*>(address);

    return { .result = value, .error = 0 };
  }

  capret_t is_permitted(capability_t* capability, uint8_t permission) {
    class_t* cap_class = lookup_class(capability->ccid);
    if (cap_class == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (permission >= cap_class->num_permissions) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    uintptr_t base  = capability->instance.value + permission / 8;
    uintptr_t value = (*reinterpret_cast<uint8_t*>(base)) & (1 << (permission % 8));

    return { .result = (uintptr_t)(value != 0), .error = 0 };
  }
} // namespace caprese::capability
