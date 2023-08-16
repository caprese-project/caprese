/**
 * @file page.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cassert>

#include <caprese/arch/memory.h>
#include <caprese/memory/page.h>

namespace caprese::memory {
  bool map(mapped_address_t root_page_table, virtual_address_t virtual_address, physical_address_t physical_address, arch::page_flags_t flags, bool allocate) {
    assert((root_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((physical_address.value & (arch::PAGE_SIZE - 1)) == 0);

    if (virtual_address.value >= CONFIG_MAX_VIRTUAL_ADDRESS) [[unlikely]] {
      return false;
    }

    return arch::map_page(root_page_table.value, virtual_address.value, physical_address.value, flags, allocate);
  }

  bool unmap(mapped_address_t root_page_table, virtual_address_t virtual_address) {
    assert((root_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::PAGE_SIZE - 1)) == 0);

    if (virtual_address.value >= CONFIG_MAX_VIRTUAL_ADDRESS) [[unlikely]] {
      return false;
    }

    return arch::unmap_page(root_page_table.value, virtual_address.value);
  }

  bool is_mapped(mapped_address_t root_page_table, virtual_address_t virtual_address) {
    assert((root_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::PAGE_SIZE - 1)) == 0);

    if (virtual_address.value >= CONFIG_MAX_VIRTUAL_ADDRESS) [[unlikely]] {
      return false;
    }

    return arch::is_mapped_page(root_page_table.value, virtual_address.value);
  }

  bool shallow_map(mapped_address_t root_page_table, virtual_address_t virtual_address) {
    assert((root_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::MAX_PAGE_SIZE - 1)) == 0);

    return arch::shallow_map_page(root_page_table.value, virtual_address.value);
  }

  bool copy_shallow_mapping(mapped_address_t dst_page_table, mapped_address_t src_page_table, virtual_address_t virtual_address) {
    assert((dst_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((src_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::MAX_PAGE_SIZE - 1)) == 0);

    return arch::copy_shallow_page_mapping(dst_page_table.value, src_page_table.value, virtual_address.value);
  }

  bool is_shallow_mapped(mapped_address_t root_page_table, virtual_address_t virtual_address) {
    assert((root_page_table.value & (arch::PAGE_SIZE - 1)) == 0);
    assert((virtual_address.value & (arch::MAX_PAGE_SIZE - 1)) == 0);

    return arch::is_shallow_mapped_page(root_page_table.value, virtual_address.value);
  }

  mapped_address_t get_mapped_address(mapped_address_t root_page_table, virtual_address_t virtual_address) {
    return physical_address_t::from(arch::get_physical_address(root_page_table.value, virtual_address.value)).mapped_address();
  }

  mapped_address_t get_current_root_page_table() {
    return physical_address_t::from(arch::get_current_root_page_table()).mapped_address();
  }
} // namespace caprese::memory
