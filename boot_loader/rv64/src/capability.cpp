/**
 * @file capability.cpp
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

#include "capability.h"

#include <bit>
#include <cassert>
#include <cerrno>
#include <cstring>

#include <caprese/arch/rv64/memory/layout.h>
#include <caprese/memory/page.h>

#include <libcaprese/util/align.h>
#include <libcaprese/util/array_size.h>

#include "memory.h"
#include "panic.h"

namespace caprese::boot_loader {
  namespace {
    capability_table_wrapper* current_table = nullptr;

    void create_new_capability_table() {
      assert(current_table == nullptr || current_table->table->used >= libcaprese::util::array_size_of(current_table->table->slots));

      auto new_table = reinterpret_cast<capability::capability_table_t*>(alloc_page());
      *new_table     = {};

      if (current_table == nullptr) [[unlikely]] {
        current_table                = reinterpret_cast<capability_table_wrapper*>(malloc(sizeof(capability_table_wrapper)));
        current_table->prev          = nullptr;
        new_table->this_cap_space_id = 1;
      } else {
        current_table->table->next_cap_space_id = current_table->table->this_cap_space_id + 1;
        new_table->prev_cap_space_id            = current_table->table->this_cap_space_id;
        new_table->this_cap_space_id            = current_table->table->next_cap_space_id;

        auto new_table_wrapper  = reinterpret_cast<capability_table_wrapper*>(malloc(sizeof(capability_table_wrapper)));
        new_table_wrapper->prev = current_table;
        current_table           = new_table_wrapper;
      }

      current_table->table = new_table;

      map_page(arch::memory::begin_of_capability_space + memory::page_size() * new_table->this_cap_space_id,
               reinterpret_cast<uintptr_t>(new_table),
               PF_R | PF_W | PF_G);
    }

    void create_memory_capabilities(const device_tree_node_t& node, uint32_t address_cells, uint32_t size_cells) {
      device_tree_node_property_t prop;
      if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
        if (errno != ENOENT) [[unlikely]] {
          panic("Failed to create memory capabilities.");
        }
        return;
      }

      errno = 0;

      uint32_t children_address_cells = 2;
      uint32_t children_size_cells    = 1;

      do {
        if (strcmp(prop.name, "reg") == 0) [[unlikely]] {
          if (strncmp(node.name, "mmode_resv", 10) == 0) [[unlikely]] {
            continue;
          }

          bool device = strncmp(node.name, "memory", 6) != 0;

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

            address = libcaprese::util::round_up(address, memory::page_size());
            size    = libcaprese::util::round_up(size, memory::page_size());

            for (size_t i = 0; i < size; i += memory::page_size()) {
              auto cap = capability::capability_memory_type {
                .valid            = 1,
                .type             = static_cast<uint64_t>(capability::capability_type::memory),
                .readable         = 1,
                .writable         = 1,
                .executable       = 1,
                .device           = device,
                .phys_page_number = (address + i) >> 12,
              };
              insert_capability(capability::capability_t { .memory = cap });
            }
          }
        } else if (strcmp(prop.name, "#address-cells") == 0) [[unlikely]] {
          children_address_cells = std::byteswap(*reinterpret_cast<const uint32_t*>(prop.data));
        } else if (strcmp(prop.name, "#size-cells") == 0) [[unlikely]] {
          children_size_cells = std::byteswap(*reinterpret_cast<const uint32_t*>(prop.data));
        }
      } while (get_next_device_tree_node_property(&prop, &prop) == 0);

      if (errno && errno != ENOENT) [[unlikely]] {
        panic("Failed to create memory capabilities.");
      }

      device_tree_node_t child_node;
      if (get_first_child_device_tree_node(&child_node, &node) < 0) [[unlikely]] {
        if (errno != ENOENT) {
          panic("Failed to create memory capabilities.");
        }
        return;
      }

      errno = 0;

      do {
        create_memory_capabilities(child_node, children_address_cells, children_size_cells);
      } while (get_next_sibling_device_tree_node(&child_node, &child_node) == 0);

      if (errno && errno != ENOENT) [[unlikely]] {
        panic("Failed to create memory capabilities.");
      }
    }

    void create_plic_irq_capability(const device_tree_node_t& node) {
      device_tree_node_property_t prop;
      if (get_first_device_tree_node_property(&prop, &node) < 0) [[unlikely]] {
        if (errno != ENOENT) [[unlikely]] {
          panic("Failed to create irq capability.");
        }
        return;
      }

      do {
        if (strcmp(prop.name, "riscv,ndev") == 0) {
          uint32_t ndev = std::byteswap(*reinterpret_cast<const uint32_t*>(prop.data));
          for (uint32_t i = 0; i < ndev; ++i) {
            auto cap = capability::capability_irq_type {
              .valid      = 1,
              .type       = static_cast<uint64_t>(capability::capability_type::irq),
              .irq_number = i,
            };
            insert_capability(capability::capability_t { .irq = cap });
          }
        }
      } while (get_next_device_tree_node_property(&prop, &prop) == 0);
    }
  } // namespace

  const capability_table_wrapper* get_current_capability_table() {
    return current_table;
  }

  void insert_capability(capability::capability_t cap) {
    if (current_table == nullptr) [[unlikely]] {
      create_new_capability_table();
    }

    if (current_table->table->used >= libcaprese::util::array_size_of(current_table->table->slots)) [[unlikely]] {
      create_new_capability_table();
    }

    current_table->table->slots[current_table->table->used] = cap;
    ++current_table->table->used;
  }

  void create_memory_capabilities(const device_tree_node_t& node) {
    create_memory_capabilities(node, 2, 1);
  }

  void create_irq_capabilities(const device_tree_node_t& node) {
    create_plic_irq_capability(node);
  }
} // namespace caprese::boot_loader
