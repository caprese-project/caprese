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

#include <caprese/memory/page.h>

namespace caprese::memory {
  bool map(virtual_address_t root_page_table, virtual_address_t virt_addr, physical_address_t phys_addr, page_flag flags) {
    assert((root_page_table.value() & page_mask()) == 0);
    assert((virt_addr.value() & page_mask()) == 0);
    assert((phys_addr.value() & page_mask()) == 0);
    return arch::memory::map(root_page_table, virt_addr, phys_addr, static_cast<uintptr_t>(flags));
  }

  bool unmap(virtual_address_t root_page_table, virtual_address_t virt_addr) {
    assert((root_page_table.value() & page_mask()) == 0);
    assert((virt_addr.value() & page_mask()) == 0);
    return arch::memory::unmap(root_page_table, virt_addr);
  }

  physical_address_t walk_page(virtual_address_t root_page_table, virtual_address_t virt_addr) {
    auto pte = arch::memory::walk_page(root_page_table, virt_addr, false);
    if (pte == nullptr) [[unlikely]] {
      return nullptr;
    }
    return arch::memory::get_page_address(pte);
  }

  void shallow_copy_mapping(virtual_address_t dst_root_page_table, virtual_address_t src_root_page_table, virtual_address_t begin, virtual_address_t end) {
    assert((dst_root_page_table.value() & page_mask()) == 0);
    assert((src_root_page_table.value() & page_mask()) == 0);
    assert((begin.value() & page_mask()) == 0);
    assert((end.value() & page_mask()) == 0);
    arch::memory::shallow_copy_mapping(dst_root_page_table, src_root_page_table, begin, end);
  }
} // namespace caprese::memory
