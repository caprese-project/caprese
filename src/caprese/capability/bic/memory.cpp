#include <bit>
#include <cstdio>
#include <cstdlib>

#include <caprese/arch/memory.h>
#include <caprese/capability/bic/memory.h>
#include <caprese/capability/bic/task.h>
#include <caprese/capability/builtin.h>
#include <caprese/memory/page.h>

namespace caprese::capability::bic::memory {
  class_t* create_class() {
    class_t* cap_class = create_capability_class();
    if (cap_class == nullptr) [[unlikely]] {
      return nullptr;
    }
    if (make_ccid(cap_class) != CCID) [[unlikely]] {
      return nullptr;
    }

    cap_class->name            = "memory";
    cap_class->flags           = CLASS_FLAG_VALID | CLASS_FLAG_BUILTIN | CLASS_FLAG_MOVABLE;
    cap_class->num_permissions = 3;
    cap_class->num_fields      = 3;
    cap_class->num_methods     = 4;
    cap_class->methods         = static_cast<method_t*>(malloc(cap_class->num_methods * sizeof(method_t)));

    if (cap_class->methods == nullptr) [[unlikely]] {
      return nullptr;
    }

    cap_class->methods[method::MAP]   = method_map;
    cap_class->methods[method::UNMAP] = method_unmap;
    cap_class->methods[method::READ]  = method_read;
    cap_class->methods[method::WRITE] = method_write;

    return cap_class;
  }

  capability_t* create(caprese::memory::physical_address_t physical_address, uint8_t flags) {
    if (physical_address.value & (arch::PAGE_SIZE - 1)) [[unlikely]] {
      return nullptr;
    }

    capability_t* cap = create_capability(CCID);
    if (cap == nullptr) [[unlikely]] {
      return nullptr;
    }

    set_permission(cap, permission::READABLE, flags & constant::READABLE);
    set_permission(cap, permission::WRITABLE, flags & constant::WRITABLE);
    set_permission(cap, permission::EXECUTABLE, flags & constant::EXECUTABLE);
    set_field(cap, field::PHYSICAL_ADDRESS, physical_address.value);
    set_field(cap, field::VIRTUAL_ADDRESS, 0);
    set_field(cap, field::TID, 0);

    return cap;
  }

  cap_ret_t method_map(capability_t* cap, uintptr_t _task_cid_handle, uintptr_t _virtual_address, uintptr_t _flags, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;
    using namespace caprese::memory;

    if (_virtual_address < CONFIG_USER_SPACE_BASE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (_virtual_address >= CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (_virtual_address & (arch::PAGE_SIZE - 1)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [current_virtual_address, current_virtual_address_error] = get_field(cap, field::VIRTUAL_ADDRESS);
    if (current_virtual_address != 0 || current_virtual_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [readable, readable_error] = is_permitted(cap, permission::READABLE);
    if (readable_error || (!readable && (static_cast<uint8_t>(_flags) & constant::READABLE))) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    auto [writable, writable_error] = is_permitted(cap, permission::WRITABLE);
    if (writable_error || (!writable && (static_cast<uint8_t>(_flags) & constant::WRITABLE))) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    auto [executable, executable_error] = is_permitted(cap, permission::EXECUTABLE);
    if (executable_error || (!executable && (static_cast<uint8_t>(_flags) & constant::EXECUTABLE))) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* current_task = get_current_task();
    cid_t*  task_cid     = lookup_cid(current_task, _task_cid_handle);
    if (task_cid == nullptr || task_cid->ccid != TASK_CAP_CCID) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    capability_t* task_cap = lookup_capability(current_task, *task_cid);
    if (task_cap == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [tid, tid_error] = get_field(task_cap, bic::task::field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    arch::page_flags_t flags {
      .readable   = (static_cast<uint8_t>(_flags) & constant::READABLE) >> std::countr_zero(constant::READABLE),
      .writable   = (static_cast<uint8_t>(_flags) & constant::WRITABLE) >> std::countr_zero(constant::WRITABLE),
      .executable = (static_cast<uint8_t>(_flags) & constant::EXECUTABLE) >> std::countr_zero(constant::EXECUTABLE),
      .user       = true,
      .global     = false,
    };

    mapped_address_t root_page_table                = get_root_page_table(task);
    auto [physical_address, physical_address_error] = get_field(cap, field::PHYSICAL_ADDRESS);
    if (physical_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    bool result = map(root_page_table, virtual_address_t::from(_virtual_address), physical_address_t::from(physical_address), flags, true);
    if (!result) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    set_field(cap, field::TID, tid);
    set_field(cap, field::VIRTUAL_ADDRESS, _virtual_address);

    return { .result = 0, .error = 0 };
  }

  cap_ret_t method_unmap(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;
    using namespace caprese::memory;

    auto [virtual_address, virtual_address_error] = get_field(cap, field::VIRTUAL_ADDRESS);
    if (virtual_address == 0 || virtual_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [physical_address, physical_address_error] = get_field(cap, field::PHYSICAL_ADDRESS);
    if (physical_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    mapped_address_t   root_page_table         = get_root_page_table(task);
    physical_address_t actual_physical_address = get_mapped_address(root_page_table, virtual_address_t::from(virtual_address)).physical_address();
    if (actual_physical_address.value != physical_address) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    bool result = unmap(root_page_table, virtual_address_t::from(virtual_address));
    if (!result) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    set_field(cap, field::VIRTUAL_ADDRESS, 0);
    set_field(cap, field::TID, 0);

    return { .result = 0, .error = 0 };
  }

  cap_ret_t method_read(capability_t* cap, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::memory;

    auto [readable, readable_error] = is_permitted(cap, permission::READABLE);
    if (readable_error || !readable) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (offset > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (size > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (offset + size > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (size > sizeof(uintptr_t)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [physical_address, physical_address_error] = get_field(cap, field::PHYSICAL_ADDRESS);
    if (physical_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    mapped_address_t page   = physical_address_t::from(physical_address).mapped_address();
    uint64_t         result = 0;
    for (size_t i = 0; i < size; ++i) {
      result |= page.as<uint8_t>()[offset + i] << (8 * i);
    }

    return { .result = result, .error = 0 };
  }

  cap_ret_t method_write(capability_t* cap, uintptr_t value, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::memory;

    auto [writable, writable_error] = is_permitted(cap, permission::WRITABLE);
    if (writable_error || !writable) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (offset > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (size > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (offset + size > arch::PAGE_SIZE) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }
    if (size > sizeof(uintptr_t)) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [physical_address, physical_address_error] = get_field(cap, field::PHYSICAL_ADDRESS);
    if (physical_address_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    mapped_address_t page = physical_address_t::from(physical_address).mapped_address();
    for (size_t i = 0; i < size; ++i) {
      page.as<uint8_t>()[offset + i] = (value >> (8 * i)) & 0xff;
    }

    return { .result = 0, .error = 0 };
  }
} // namespace caprese::capability::bic::memory
