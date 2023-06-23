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
#include <cstring>

#include <caprese/arch/common/memory/page.h>
#include <caprese/memory/page.h>
#include <caprese/memory/page_stack.h>

namespace caprese::arch::memory {
  namespace {
    constexpr int get_index(caprese::memory::virtual_address_t virt_addr, int level) {
      return (virt_addr.value() >> (9 * level + PAGE_SIZE_BIT)) & 0x1FF;
    }

    constexpr uintptr_t SATP_PAGE_NUMBER_MASK = (1ull << 44) - 1;
  } // namespace

  page_table_t walk_page(caprese::memory::virtual_address_t root_page_table, caprese::memory::virtual_address_t virt_addr, bool alloc) {
    using namespace caprese::memory;

    auto page_table = root_page_table.as<page_table_t>();

    for (int i = 3; i > 0; --i) {
      auto& pte = page_table[get_index(virt_addr, i)];
      if (pte.v == 0) [[unlikely]] {
        if (!alloc) [[unlikely]] {
          return nullptr;
        }
        pte.v               = 1;
        auto next_page_addr = capability::get_capability(caprese::memory::pop_page()).memory.address();
        memset(next_page_addr.as<void*>(), 0, caprese::memory::page_size());
        pte.next_page_number = virt_to_phys(next_page_addr).value() >> PAGE_SIZE_BIT;
      }
      page_table = phys_to_virt(pte.next_page_number << PAGE_SIZE_BIT).as<page_table_t>();
    }

    return &page_table[get_index(virt_addr, 0)];
  }

  bool map(caprese::memory::virtual_address_t  root_page_table,
           caprese::memory::virtual_address_t  virt_addr,
           caprese::memory::physical_address_t phys_addr,
           uintptr_t                           flags) {
    auto pte = walk_page(root_page_table, virt_addr, true);
    if (pte == nullptr || pte->v != 0) [[unlikely]] {
      return false;
    }

    *reinterpret_cast<uintptr_t*>(pte) = flags;
    pte->v                             = 1;
    pte->next_page_number              = phys_addr.value() >> PAGE_SIZE_BIT;

    return true;
  }

  bool unmap(caprese::memory::virtual_address_t root_page_table, caprese::memory::virtual_address_t virt_addr) {
    auto pte = walk_page(root_page_table, virt_addr, false);
    if (pte == nullptr || pte->v == 0) [[unlikely]] {
      return false;
    }

    *pte = {};

    return true;
  }

  void shallow_copy_mapping(caprese::memory::virtual_address_t dst_root_page_table,
                            caprese::memory::virtual_address_t src_root_page_table,
                            caprese::memory::virtual_address_t begin,
                            caprese::memory::virtual_address_t end) {
    auto dst = dst_root_page_table.as<page_table_t>();
    auto src = src_root_page_table.as<page_table_t>();
    for (uintptr_t page = begin.value(); page < end.value(); page += (1ull << (9 * 3 + 12))) {
      dst[get_index(page, 3)] = src[get_index(page, 3)];
    }
  }
} // namespace caprese::arch::memory
