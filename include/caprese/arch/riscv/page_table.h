/**
 * @file page_table.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declares functions related to page tables.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RISCV_PAGE_TABLE_H_
#define CAPRESE_ARCH_RISCV_PAGE_TABLE_H_

#ifdef CONFIG_ARCH_RISCV

#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include <caprese/kernel/types.h>
#include <caprese/lib/console.h>
#include <caprese/lib/error.h>

namespace caprese::arch {
  constexpr uintptr_t PAGE_SIZE           = 4096;
  constexpr uintptr_t MAX_VIRTUAL_ADDRESS = 1ull << (9 + 9 + 9 + 12 - 1);

  struct sv39_page_table_entry {
    uint64_t v       : 1;  // valid
    uint64_t r       : 1;  // readable
    uint64_t w       : 1;  // writable
    uint64_t x       : 1;  // executable
    uint64_t u       : 1;  // user page
    uint64_t g       : 1;  // global
    uint64_t a       : 1;  // accessed
    uint64_t d       : 1;  // defaced
    uint64_t rsw     : 2;  // reserved
    uint64_t ppn0    : 9;  // physical page number 0
    uint64_t ppn1    : 9;  // physical page number 1
    uint64_t ppn2    : 26; // physical page number 2
    uint64_t reserved: 10; // reserved
  };

  struct sv39_virtual_address {
    uint64_t page_offset: 12;
    uint64_t vpn0       : 9;
    uint64_t vpn1       : 9;
    uint64_t vpn2       : 9;
  };

  struct sv39_physical_address {
    uint64_t page_offset: 12;
    uint64_t ppn0       : 9;
    uint64_t ppn1       : 9;
    uint64_t ppn2       : 26;
  };

  static_assert(sizeof(sv39_page_table_entry) == sizeof(uint64_t));
  static_assert(sizeof(sv39_virtual_address) == sizeof(virtual_address_t));
  static_assert(sizeof(sv39_physical_address) == sizeof(physical_address_t));

  inline physical_address_t page_table_entry_to_physical_address(const sv39_page_table_entry& page_table_entry) {
    return std::bit_cast<physical_address_t>(page_table_entry) >> 10 << 12;
  }

  inline sv39_page_table_entry physical_address_to_page_table_entry(const physical_address_t& physical_address) {
    return std::bit_cast<sv39_page_table_entry>(physical_address >> 12 << 10);
  }

  struct page_region_info {
    virtual_address_t  base_virtual_address;
    physical_address_t begin_of_physical_address;
    physical_address_t end_of_physical_address;
    bool               readable;
    bool               writable;
    bool               executable;
    bool               user;
  };

  class early_page_table {
    sv39_page_table_entry* satp_page_table_entry;

    sv39_page_table_entry* base_of_page_table_entry_2;
    sv39_page_table_entry* base_of_page_table_entry_1;
    sv39_page_table_entry* base_of_page_table_entry_0;

  public:
    error_t   enable_mmu();
    uintptr_t get_satp_value();

    template<size_t N>
    error_t create(physical_address_t begin_of_page_table, physical_address_t end_of_page_table, const page_region_info (&regions)[N]) {
      assert(begin_of_page_table % PAGE_SIZE == 0);
      assert(end_of_page_table % PAGE_SIZE == 0);

#ifndef NDEBUG
      {
        virtual_address_t prev_virtual_address = 0;
        for (const page_region_info& region : regions) {
          assert(region.base_virtual_address % PAGE_SIZE == 0);
          assert(region.begin_of_physical_address % PAGE_SIZE == 0);
          assert(prev_virtual_address < region.base_virtual_address);
          prev_virtual_address = region.base_virtual_address + (region.end_of_physical_address - region.begin_of_physical_address) - 1;
        }
      }
#endif // NDEBUG

      satp_page_table_entry = reinterpret_cast<sv39_page_table_entry*>(begin_of_page_table);

      {
        std::bitset<1 << 9> vpn2;
        for (const page_region_info& region : regions) {
          const size_t size_of_address_space = region.end_of_physical_address - region.begin_of_physical_address;

          for (virtual_address_t offset = 0; offset < size_of_address_space; offset += PAGE_SIZE) {
            vpn2[std::bit_cast<sv39_virtual_address>(region.base_virtual_address + offset).vpn2] = true;
          }
        }

        base_of_page_table_entry_2 = satp_page_table_entry;
        base_of_page_table_entry_1 = base_of_page_table_entry_2 + (1 << 9);
        base_of_page_table_entry_0 = base_of_page_table_entry_1 + (1 << 9) * vpn2.count();
      }

      log_debug("early_page_table", "base_of_page_table_entry_2: %p", base_of_page_table_entry_2);
      log_debug("early_page_table", "base_of_page_table_entry_1: %p", base_of_page_table_entry_1);
      log_debug("early_page_table", "base_of_page_table_entry_0: %p", base_of_page_table_entry_0);

      {
        std::bitset<1 << 9> vpn2;
        std::bitset<1 << 9> vpn1;
        std::bitset<1 << 9> vpn0;
        size_t              vpn1_count = 0;
        for (const page_region_info& region : regions) {
          const size_t size_of_address_space = region.end_of_physical_address - region.begin_of_physical_address;

          for (virtual_address_t offset = 0; offset < size_of_address_space; offset += PAGE_SIZE) {
            const auto             virtual_address    = std::bit_cast<sv39_virtual_address>(region.base_virtual_address + offset);
            const auto             physical_address   = region.begin_of_physical_address + offset;
            sv39_page_table_entry* page_table_entry_2 = nullptr;
            sv39_page_table_entry* page_table_entry_1 = nullptr;
            sv39_page_table_entry* page_table_entry_0 = nullptr;

            if (!vpn2[virtual_address.vpn2]) [[unlikely]] {
              vpn1.reset();
              vpn2[virtual_address.vpn2] = true;
            }
            if (!vpn1[virtual_address.vpn1]) [[unlikely]] {
              ++vpn1_count;
              vpn1[virtual_address.vpn1] = true;
            }

            page_table_entry_2 = base_of_page_table_entry_2 + virtual_address.vpn2;

            if (page_table_entry_2->v == 0) [[unlikely]] {
              page_table_entry_1    = base_of_page_table_entry_1 + (1 << 9) * (vpn2.count() - 1);
              *page_table_entry_2   = physical_address_to_page_table_entry(reinterpret_cast<physical_address_t>(page_table_entry_1));
              page_table_entry_2->v = 1;
            } else {
              page_table_entry_1 = reinterpret_cast<sv39_page_table_entry*>(page_table_entry_to_physical_address(*page_table_entry_2));
            }

            page_table_entry_1 += virtual_address.vpn1;

            if (page_table_entry_1->v == 0) [[unlikely]] {
              page_table_entry_0    = base_of_page_table_entry_0 + (1 << 9) * (vpn1_count - 1);
              *page_table_entry_1   = physical_address_to_page_table_entry(reinterpret_cast<physical_address_t>(page_table_entry_0));
              page_table_entry_1->v = 1;
            } else {
              page_table_entry_0 = reinterpret_cast<sv39_page_table_entry*>(page_table_entry_to_physical_address(*page_table_entry_1));
            }

            page_table_entry_0 += virtual_address.vpn0;

            log_debug("early_page_table", "page_table_entry_2: %p", page_table_entry_2);
            log_debug("early_page_table", "page_table_entry_1: %p", page_table_entry_1);
            log_debug("early_page_table", "page_table_entry_0: %p", page_table_entry_0);

            if (page_table_entry_0->v == 1) [[unlikely]] {
              log_error("early_page_table", "Already mapped. virtual_address: %p, physical_address: %p", virtual_address, physical_address);
              return error_t::INVALID_ARGUMENT;
            }

            *page_table_entry_0   = physical_address_to_page_table_entry(physical_address);
            page_table_entry_0->v = 1;
            page_table_entry_0->r = region.readable;
            page_table_entry_0->w = region.writable;
            page_table_entry_0->x = region.executable;
            page_table_entry_0->u = region.user;

            log_info("early_page_table",
                     "Mapped memory: virtual address: %p, physical address: %p, R: %c, W: %c, X: %c, U: %c",
                     virtual_address,
                     physical_address,
                     region.readable ? 'T' : 'F',
                     region.writable ? 'T' : 'F',
                     region.executable ? 'T' : 'F',
                     region.user ? 'T' : 'F');
          }
        }
      }

      return error_t::OK;
    }
  };
} // namespace caprese::arch

#endif // CONFIG_ARCH_RISCV

#endif // CAPRESE_ARCH_RISCV_PAGE_TABLE_H_
