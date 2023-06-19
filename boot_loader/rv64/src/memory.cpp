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

#include "memory.h"

#include <bit>
#include <cassert>
#include <cerrno>
#include <cstring>

#include "panic.h"

namespace caprese::boot_loader {
  namespace {
    page_table_t root_page_table;

    struct arena {
      arena*        next_arena;
      memory_region region;
      uintptr_t     allocated;
    };

    arena* root_arena;

    arena*
        create_arena(const device_tree_node_t& node, uintptr_t free_page_start, arena* current = nullptr, uint32_t address_cells = 2, uint32_t size_cells = 1) {
      if (strncmp(node.name, "memory", 6) == 0) [[unlikely]] {
        device_tree_node_property_t prop;
        if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
          if (errno != ENOENT) [[unlikely]] {
            panic("Failed to create arena.");
          }
        } else {
          do {
            if (strcmp(prop.name, "reg") == 0) [[unlikely]] {
              auto ptr = reinterpret_cast<const uint32_t*>(prop.data);
              auto end = reinterpret_cast<const uint32_t*>(prop.data + prop.size_of_data);

              while (ptr < end) {
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

                memory_region region { .begin = address, .end = address + size };

                if (region.begin <= free_page_start && free_page_start < region.end) [[unlikely]] {
                  region.begin = free_page_start;
                }

                if (current == nullptr) [[unlikely]] {
                  root_arena = current = reinterpret_cast<arena*>(malloc(sizeof(arena)));
                } else {
                  current->next_arena = reinterpret_cast<arena*>(malloc(sizeof(arena)));
                  current             = current->next_arena;
                }
                current->region     = region;
                current->allocated  = region.begin;
                current->next_arena = nullptr;
              }
            }
          } while (get_next_device_tree_node_property(&prop, &prop) == 0);
        }
      }

      uint32_t children_address_cells = 2;
      uint32_t children_size_cells    = 1;

      device_tree_node_property_t prop;
      if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
        if (errno != ENOENT) [[unlikely]] {
          panic("Failed to create arena.");
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
          panic("Failed to create arena.");
        }
      }

      device_tree_node_t child_node;
      if (get_first_child_device_tree_node(&child_node, &node) < 0) [[unlikely]] {
        return current;
      }

      errno = 0;

      do {
        current = create_arena(child_node, free_page_start, current, children_address_cells, children_size_cells);
      } while (get_next_sibling_device_tree_node(&child_node, &child_node) == 0);

      if (errno && errno != ENOENT) [[unlikely]] {
        panic("Failed to load device tree blob.");
      }

      return current;
    }
  } // namespace

  void init_free_page(const device_tree_node_t& node, uintptr_t free_page_start) {
    create_arena(node, free_page_start);
  }

  uintptr_t alloc_page() {
    auto arena = root_arena;
    while (arena) {
      if (arena->region.end <= arena->allocated) [[unlikely]] {
        arena = arena->next_arena;
      } else {
        uintptr_t result = arena->allocated;
        arena->allocated += 0x1000;
        return result;
      }
    }
    panic("Out of memory.");
  }

  void create_root_page_table() {
    root_page_table = reinterpret_cast<page_table_t>(alloc_page());
  }

  uintptr_t get_root_page_table() {
    return reinterpret_cast<uintptr_t>(root_page_table);
  }

  void map_page(uintptr_t virtual_address, uintptr_t physical_address, uint64_t flags) {
    if (virtual_address & 0x1FF) [[unlikely]] {
      panic("virtual_address is not aligned with page size: %p", virtual_address);
    }
    if (physical_address & 0x1FF) [[unlikely]] {
      panic("physical_address is not aligned with page size: %p", physical_address);
    }

    const auto get_index = [virtual_address](int level) {
      return (virtual_address >> (9 * level + 12)) & 0x1FF;
    };

    page_table_t page_table = root_page_table;
    for (int i = 3; i > 0; --i) {
      auto& pte = page_table[get_index(i)];
      if (pte.v == 0) [[unlikely]] {
        *reinterpret_cast<uint64_t*>(&pte) = 0;
        pte.v                              = 1;
        pte.next_page_number               = alloc_page() >> 12;
      }
      page_table = reinterpret_cast<page_table_t>(pte.next_page_number << 12);
    }

    auto& pte = page_table[get_index(0)];
    if (pte.v != 0) [[unlikely]] {
      panic("Already mapped: %p", reinterpret_cast<void*>(virtual_address));
    }

    *reinterpret_cast<uint64_t*>(&pte) = flags;
    pte.v                              = 1;
    pte.next_page_number               = physical_address >> 12;
  }

  void enable_mmu() {
    uintptr_t satp = reinterpret_cast<uintptr_t>(root_page_table) >> 12;
    satp |= 9ull << 60;
    asm volatile("sfence.vma zero, zero");
    asm volatile("csrw satp, %0" : : "r"(satp));
    asm volatile("sfence.vma zero, zero");
  }
} // namespace caprese::boot_loader

namespace {
  uint8_t early_memory_pool[0x1000];
  size_t  early_memory_pool_used = 0;

  struct heap_t {
    caprese::boot_loader::arena* arena;
    size_t                       allocated;
  };

  heap_t* heap;
} // namespace

extern "C" void* malloc(size_t size) {
  if (early_memory_pool_used + size < sizeof(early_memory_pool)) {
    auto result = early_memory_pool + early_memory_pool_used;
    early_memory_pool_used += size;
    return result;
  }

  if (heap == nullptr) [[unlikely]] {
    auto arena = caprese::boot_loader::root_arena;
    while (arena->next_arena) {
      arena = arena->next_arena;
    }
    heap            = reinterpret_cast<heap_t*>(arena->region.end - sizeof(heap_t));
    heap->arena     = arena;
    heap->allocated = sizeof(heap_t);
  }

  auto ptr = heap->arena->region.end - heap->allocated - size;
  if (ptr <= heap->arena->region.begin) [[unlikely]] {
    panic("Out of memory");
  }

  heap->allocated += size;

  return reinterpret_cast<void*>(ptr);
}
