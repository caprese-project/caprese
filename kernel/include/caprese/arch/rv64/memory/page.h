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

#ifndef CAPRESE_ARCH_RV64_MEMORY_PAGE_H_
#define CAPRESE_ARCH_RV64_MEMORY_PAGE_H_

#include <cstddef>
#include <cstdint>

#include <caprese/memory/address.h>

namespace caprese::arch::memory {
  constexpr size_t PAGE_SIZE_BIT = 12;

  constexpr int PAGE_FLAG_READABLE_BIT   = 1 << 1;
  constexpr int PAGE_FLAG_WRITABLE_BIT   = 1 << 2;
  constexpr int PAGE_FLAG_EXECUTABLE_BIT = 1 << 3;
  constexpr int PAGE_FLAG_USER_MODE_BIT  = 1 << 4;
  constexpr int PAGE_FLAG_GLOBAL_BIT     = 1 << 5;

  struct page_table_entry {
    uint64_t v               : 1;
    uint64_t r               : 1;
    uint64_t w               : 1;
    uint64_t x               : 1;
    uint64_t u               : 1;
    uint64_t g               : 1;
    uint64_t a               : 1;
    uint64_t d               : 1;
    uint64_t rsv             : 2;
    uint64_t next_page_number: 44;
  };

  using page_table_t = page_table_entry*;

  page_table_t walk_page(caprese::memory::virtual_address_t root_page_table, caprese::memory::virtual_address_t virt_addr, bool alloc);

  inline caprese::memory::physical_address_t get_page_address(page_table_t page_table) {
    return page_table->next_page_number << 12;
  }
} // namespace caprese::arch::memory

#endif // CAPRESE_ARCH_RV64_MEMORY_PAGE_H_
