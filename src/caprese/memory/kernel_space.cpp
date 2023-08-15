#include <cstdlib>

#include <caprese/memory/kernel_space.h>
#include <caprese/memory/page.h>
#include <caprese/util/panic.h>

namespace caprese::memory {
  bool init_kernel_space() {
    mapped_address_t root_page_table = get_current_root_page_table();
    for (uintptr_t offset = 0; offset < CONFIG_ACTUAL_KERNEL_SPACE_SIZE; offset += arch::MAX_PAGE_SIZE) {
      virtual_address_t virtual_address = virtual_address_t::from(CONFIG_KERNEL_SPACE_BASE + offset);
      if (!is_shallow_mapped(root_page_table, virtual_address)) {
        mapped_address_t page = mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
        if (page.is_null()) [[unlikely]] {
          return false;
        }

        bool result = shallow_map(root_page_table, virtual_address, page.physical_address());
        if (!result) [[unlikely]] {
          return false;
        }
      }
    }
    return true;
  }

  bool copy_kernel_space_mapping(mapped_address_t dst_page_table, mapped_address_t src_page_table) {
    for (uintptr_t offset = 0; offset < CONFIG_ACTUAL_KERNEL_SPACE_SIZE; offset += arch::MAX_PAGE_SIZE) {
      virtual_address_t virtual_address = virtual_address_t::from(CONFIG_KERNEL_SPACE_BASE + offset);
      bool              result          = copy_shallow_mapping(dst_page_table, src_page_table, virtual_address);
      if (!result) [[unlikely]] {
        return false;
      }
    };
    return true;
  }
} // namespace caprese::memory
