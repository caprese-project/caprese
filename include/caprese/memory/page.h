/**
 * @file page.h
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

#ifndef CAPRESE_MEMORY_PAGE_H_
#define CAPRESE_MEMORY_PAGE_H_

#include <caprese/arch/common/memory/page.h>
#include <caprese/memory/address.h>

namespace caprese::memory {
  enum struct page_flag {
    readable   = arch::memory::PAGE_FLAG_READABLE_BIT,
    writable   = arch::memory::PAGE_FLAG_WRITABLE_BIT,
    executable = arch::memory::PAGE_FLAG_EXECUTABLE_BIT,
    user_mode  = arch::memory::PAGE_FLAG_USER_MODE_BIT,
    global     = arch::memory::PAGE_FLAG_GLOBAL_BIT,
  };

  constexpr size_t page_size() {
    return 1 << arch::memory::PAGE_SIZE_BIT;
  }

  constexpr size_t page_size_bit() {
    return arch::memory::PAGE_SIZE_BIT;
  }

  constexpr size_t page_mask() {
    return page_size() - 1;
  }

  bool map(virtual_address_t root_page_table, virtual_address_t virt_addr, physical_address_t phys_addr, page_flag flags);
  bool unmap(virtual_address_t root_page_table, virtual_address_t virt_addr);

  physical_address_t walk_page(virtual_address_t root_page_table, virtual_address_t virt_addr);

  void shallow_copy_mapping(virtual_address_t dst_root_page_table, virtual_address_t src_root_page_table, virtual_address_t begin, virtual_address_t end);
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_PAGE_H_
