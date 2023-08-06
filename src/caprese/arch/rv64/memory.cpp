/**
 * @file memory.cpp
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

#include <cstdlib>

#include <caprese/arch/rv64/memory.h>

namespace caprese::arch::inline rv64 {
  namespace {
    struct page_table_entry_t {
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

    page_table_entry_t* walk_page(uintptr_t root_page_table, uintptr_t virtual_address, bool allocate) {
#if defined(CONFIG_MMU_SV39)
      constexpr int levels = 2;
#elif defined(CONFIG_MMU_SV48)
      constexpr int levels = 3;
#endif

      const auto get_index = [&virtual_address](int level) {
        return (virtual_address >> (9 * level + PAGE_SIZE_BIT)) & 0x1ff;
      };

      page_table_entry_t* page_table = reinterpret_cast<page_table_entry_t*>(root_page_table);

      for (int i = levels; i > 0; --i) {
        page_table_entry_t* pte = page_table + get_index(i);
        if (pte->v == 0) {
          if (!allocate) [[unlikely]] {
            return nullptr;
          }
          *pte = {
            .v                = 1,
            .r                = 0,
            .w                = 0,
            .x                = 0,
            .u                = 0,
            .g                = 0,
            .a                = 0,
            .d                = 0,
            .rsv              = 0,
            .next_page_number = reinterpret_cast<uint64_t>(aligned_alloc(PAGE_SIZE, PAGE_SIZE)) >> PAGE_SIZE_BIT,
          };
          if (!pte->next_page_number) [[unlikely]] {
            return nullptr;
          }
        }
        page_table = reinterpret_cast<page_table_entry_t*>(pte->next_page_number << PAGE_SIZE_BIT);
      }

      return page_table + get_index(0);
    }
  } // namespace

  bool map_page(uintptr_t root_page_table, uintptr_t virtual_address, uintptr_t physical_address, page_flags_t flags, bool allocate) {
    page_table_entry_t* pte = walk_page(root_page_table, virtual_address, allocate);
    if (pte == nullptr || pte->v) [[unlikely]] {
      return false;
    }

    pte->v                = 1;
    pte->r                = flags.readable;
    pte->w                = flags.writable;
    pte->x                = flags.executable;
    pte->u                = flags.user;
    pte->next_page_number = physical_address >> PAGE_SIZE_BIT;

    return true;
  }

  bool unmap_page(uintptr_t root_page_table, uintptr_t virtual_address) {
    page_table_entry_t* pte = walk_page(root_page_table, virtual_address, false);
    if (pte == nullptr || !pte->v) [[unlikely]] {
      return false;
    }

    *pte = {};

    return true;
  }
} // namespace caprese::arch::inline rv64
