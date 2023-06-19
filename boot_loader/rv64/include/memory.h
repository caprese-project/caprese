/**
 * @file memory.h
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

#ifndef BOOT_LOADER_MEMORY_H_
#define BOOT_LOADER_MEMORY_H_

#include <cstddef>
#include <cstdint>

#include <libcaprese/device/device_tree.h>

namespace caprese::boot_loader {
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

  using page_table_t = page_table_entry_t*;

  constexpr uint64_t PF_R = 1 << 1;
  constexpr uint64_t PF_W = 1 << 2;
  constexpr uint64_t PF_X = 1 << 3;
  constexpr uint64_t PF_U = 1 << 4;
  constexpr uint64_t PF_G = 1 << 5;

  struct memory_region {
    uintptr_t begin;
    uintptr_t end;
  };

  void      init_free_page(const device_tree_node_t& node, uintptr_t free_page_start);
  uintptr_t alloc_page();

  void      create_root_page_table();
  uintptr_t get_root_page_table();
  void      map_page(uintptr_t virtual_address, uintptr_t physical_address, uint64_t flags);

  void enable_mmu();
} // namespace caprese::boot_loader

extern "C" void* malloc(size_t size);

#endif // BOOT_LOADER_MEMORY_H_
