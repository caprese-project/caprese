/**
 * @file start.cpp
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

#include <bit>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <caprese/memory/page.h>

#include <libcaprese/device/device_tree.h>
#include <libcaprese/util/align.h>

#include "boot_info.h"
#include "capability.h"
#include "memory.h"
#include "panic.h"

extern "C" {
  extern char _boot_loader_start[];
  extern char _boot_loader_end[];
  extern char _kernel_start[];
  extern char _kernel_end[];
}

namespace caprese::boot_loader {
  void map_kernel_space() {
    auto phys_base = reinterpret_cast<uintptr_t>(_kernel_start);
    auto virt_base = arch::memory::begin_of_kernel_code_space;
    auto size      = _kernel_end - _kernel_start;
    for (decltype(size) i = 0; i < size; i += memory::page_size()) {
      map_page(virt_base + i, phys_base + i, PF_R | PF_W | PF_X | PF_G);
    }
  }

  void map_phys_space() {
    auto cap_table = get_current_capability_table();
    while (cap_table) {
      for (uint8_t i = 0; i < cap_table->table->used; ++i) {
        if (cap_table->table->slots[i].type() == capability::capability_type::memory) [[likely]] {
          auto phys_addr = cap_table->table->slots[i].memory.phys_page_number << memory::page_size_bit();
          auto virt_addr = arch::memory::begin_of_phys_map_space + phys_addr;
          if (virt_addr > arch::memory::end_of_phys_map_space) [[unlikely]] {
            continue;
          }
          map_page(virt_addr, phys_addr, PF_R | PF_W | PF_G);
        }
      }
      cap_table = cap_table->prev;
    }
  }

  void map_boot_loader_space() {
    auto phys_base = reinterpret_cast<uintptr_t>(_boot_loader_start);
    auto virt_base = phys_base;
    auto size      = _boot_loader_end - _boot_loader_start;
    for (decltype(size) i = 0; i < size; i += memory::page_size()) {
      map_page(virt_base + i, phys_base + i, PF_R | PF_W | PF_X);
    }
  }

  memory_region get_mmode_resv(const device_tree_node_t& node, uint32_t address_cells = 2, uint32_t size_cells = 1) {
    if (strncmp(node.name, "mmode_resv", 10) == 0) [[unlikely]] {
      device_tree_node_property_t prop;
      if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
        if (errno != ENOENT) [[unlikely]] {
          panic("Failed to get mmode resv.");
        }
      } else {
        do {
          if (strcmp(prop.name, "reg") == 0) [[unlikely]] {
            auto ptr = reinterpret_cast<const uint32_t*>(prop.data);

            uintptr_t address = 0;
            for (size_t i = 0; i < address_cells; ++i) {
              uint32_t cell = std::byteswap(*ptr);
              address <<= 32;
              address |= cell;
              ++ptr;
            }

            size_t size = 0;
            for (size_t i = 0; i < size_cells; ++i) {
              uint32_t cell = std::byteswap(*ptr);
              size <<= 32;
              size |= cell;
              ++ptr;
            }

            return { address, address + size };
          }
        } while (get_next_device_tree_node_property(&prop, &prop) == 0);
      }
    }

    uint32_t children_address_cells = 2;
    uint32_t children_size_cells    = 1;

    device_tree_node_property_t prop;
    if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
      if (errno != ENOENT) [[unlikely]] {
        panic("Failed to get mmode resv.");
      }
    } else {
      errno = 0;

      do {
        if (strcmp(prop.name, "#address-cells") == 0) [[unlikely]] {
          children_address_cells = std::byteswap(*reinterpret_cast<const uint32_t*>(prop.data));
        } else if (strcmp(prop.name, "#size-cells") == 0) [[unlikely]] {
          children_size_cells = std::byteswap(*reinterpret_cast<const uint32_t*>(prop.data));
        }
      } while (get_next_device_tree_node_property(&prop, &prop) == 0);

      if (errno && errno != ENOENT) [[unlikely]] {
        panic("Failed to get mmode resv.");
      }
    }

    device_tree_node_t child_node;
    if (get_first_child_device_tree_node(&child_node, &node) < 0) [[unlikely]] {
      return { 0, 0 };
    }

    errno = 0;

    do {
      auto result = get_mmode_resv(child_node, children_address_cells, children_size_cells);
      if (result.end != 0) [[unlikely]] {
        return result;
      }
    } while (get_next_sibling_device_tree_node(&child_node, &child_node) == 0);

    if (errno && errno != ENOENT) [[unlikely]] {
      panic("Failed to get mmode resv.");
    }

    return { 0, 0 };
  }
} // namespace caprese::boot_loader

extern "C" [[noreturn]] void start(uint64_t hartid, const char* device_tree_blob) {
  device_tree_t dev_tree;
  if (load_device_tree_blob(&dev_tree, device_tree_blob) < 0) [[unlikely]] {
    panic("Failed to load device tree blob.");
  }

  device_tree_node_t root_node;
  if (get_root_device_tree_node(&root_node, &dev_tree) < 0) [[unlikely]] {
    panic("Failed to get root device tree node.");
  }

  auto free_page_start = libcaprese::util::round_up(reinterpret_cast<uintptr_t>(device_tree_blob) + dev_tree.total_size, caprese::memory::page_size());
  caprese::boot_loader::init_free_page(root_node, free_page_start);
  caprese::boot_loader::create_root_page_table();

  caprese::boot_loader::create_memory_capabilities(root_node);
  caprese::boot_loader::create_irq_capabilities(root_node);

  caprese::boot_loader::shallow_map_huge_page();

  caprese::boot_loader::map_kernel_space();
  caprese::boot_loader::map_phys_space();
  caprese::boot_loader::map_boot_loader_space();

  auto mmode_resv = caprese::boot_loader::get_mmode_resv(root_node);

  auto boot_info               = reinterpret_cast<caprese::boot_loader::boot_info_t*>(caprese::boot_loader::alloc_page());
  boot_info->hartid            = hartid;
  boot_info->device_tree_blob  = caprese::arch::memory::begin_of_phys_map_space + reinterpret_cast<uintptr_t>(device_tree_blob);
  boot_info->root_page_table   = caprese::arch::memory::begin_of_phys_map_space + caprese::boot_loader::get_root_page_table();
  boot_info->nhart             = 1; // TODO: impl
  boot_info->total_memory_size = caprese::boot_loader::get_total_memory_size();

  size_t     used_regions_count = 0;
  const auto insert_region      = [boot_info, &used_regions_count](uintptr_t begin, uintptr_t end) {
    boot_info->used_regions[used_regions_count].begin = begin;
    boot_info->used_regions[used_regions_count].end   = end;
    ++used_regions_count;
    boot_info->used_regions[used_regions_count].begin = 0;
    boot_info->used_regions[used_regions_count].end   = 0;
  };

  insert_region(reinterpret_cast<uintptr_t>(_boot_loader_start), reinterpret_cast<uintptr_t>(_boot_loader_end));
  insert_region(reinterpret_cast<uintptr_t>(_kernel_start), reinterpret_cast<uintptr_t>(_kernel_end));
  insert_region(mmode_resv.begin, mmode_resv.end);
  auto arena = caprese::boot_loader::root_arena;
  while (arena) {
    if (arena->allocated > 0) {
      insert_region(arena->region.begin, arena->region.begin + arena->allocated);
    }
    arena = arena->next_arena;
  }

  caprese::boot_loader::enable_mmu();

  asm volatile("mv a0, %0" : : "r"(caprese::arch::memory::begin_of_phys_map_space + reinterpret_cast<uintptr_t>(boot_info)));
  asm volatile("jr %0" : : "r"(caprese::arch::memory::begin_of_kernel_code_space));

  panic("UNREACHABLE");
}
