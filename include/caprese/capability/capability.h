/**
 * @file capability.h
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

#ifndef CAPRESE_CAPABILITY_CAPABILITY_H_
#define CAPRESE_CAPABILITY_CAPABILITY_H_

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <caprese/arch/common/memory/layout.h>
#include <caprese/memory/page.h>

namespace caprese::capability {
  enum struct capability_type {
    null         = 0,
    parent       = 1,
    memory       = 2,
    irq          = 3,
    rsv4         = 4,
    rsv5         = 5,
    rsv6         = 6,
    rsv7         = 7,
    rsv8         = 8,
    rsv9         = 9,
    rsv10        = 10,
    rsv11        = 11,
    rsv12        = 12,
    rsv13        = 13,
    rsv14        = 14,
    rsv15        = 15,
    rsv16        = 16,
    rsv17        = 17,
    rsv18        = 18,
    rsv19        = 19,
    rsv20        = 20,
    rsv21        = 21,
    rsv22        = 22,
    rsv23        = 23,
    rsv24        = 24,
    rsv25        = 25,
    rsv26        = 26,
    rsv27        = 27,
    rsv28        = 28,
    rsv29        = 29,
    rsv30        = 30,
    user_defined = 31,
  };

  union capability_t;

  struct capability_null_type {
    uint64_t valid    : 1;
    uint64_t type     : 5;
    uint64_t reserved1: 58;
    uint64_t reserved2;
  };

  struct capability_parent_type {
    uint64_t valid    : 1;
    uint64_t type     : 5;
    uint64_t rsv      : 10;
    uint64_t node_page: 48;
  };

  struct capability_memory_type {
    uint64_t valid           : 1;
    uint64_t type            : 5;
    uint64_t readable        : 1;
    uint64_t writable        : 1;
    uint64_t executable      : 1;
    uint64_t device          : 1;
    uint64_t phys_page_number: 36;
    uint64_t reserved0       : 18;
    uint64_t reserved1       : 64;

    inline memory::virtual_address_t address() {
      return memory::phys_to_virt(phys_page_number << memory::page_size_bit());
    }
  };

  struct capability_irq_type {
    uint64_t valid     : 1;
    uint64_t type      : 5;
    uint64_t irq_number: 32;
    uint64_t reserved1 : 26;
    uint64_t reserved2 : 64;
  };

  struct capability_user_defined_type {
    uint64_t valid : 1;
    uint64_t type  : 5;
    uint64_t id    : 24;
    uint64_t value1: 34;
    uint64_t value2: 64;
  };

  struct capability_node;

  union capability_t {
    struct {
      uint64_t valid: 1;
      uint64_t type : 5;
    } _header;

    capability_null_type         null;
    capability_parent_type       parent;
    capability_memory_type       memory;
    capability_irq_type          irq;
    capability_user_defined_type user_defined;

    constexpr bool valid() const {
      return _header.valid == 1;
    }

    constexpr capability_type type() const {
      return static_cast<capability_type>(_header.type);
    }
  };

  static_assert(sizeof(capability_t) == sizeof(uint64_t) * 2);

  struct capability_handle_t {
    uint64_t table_index        : 8;
    uint64_t generation         : 24;
    uint64_t capability_space_id: 32;
  };

  struct capability_table_t {
    uint32_t prev_cap_space_id;
    uint32_t next_cap_space_id;
    uint64_t task_id;
    uint8_t  used;
    uint8_t  released;
    uint16_t reserved0;
    uint32_t generation: 24;
    uint32_t this_cap_space_id;
    uint32_t reserved1;
    alignas(sizeof(capability_t)) capability_t slots[memory::page_size() / sizeof(capability_t) - 2];
  };

  static_assert(sizeof(capability_table_t) == memory::page_size());

  inline capability_table_t* get_capability_table_by_id(uint32_t cap_space_id) {
    return reinterpret_cast<capability_table_t*>(arch::memory::begin_of_capability_space + cap_space_id * sizeof(capability_table_t));
  }

  inline capability_t& get_capability(capability_handle_t handle) {
    return get_capability_table_by_id(handle.capability_space_id)->slots[handle.table_index];
  }

  inline capability_handle_t make_handle(const capability_t* cap) {
    assert(reinterpret_cast<uintptr_t>(cap) >= arch::memory::begin_of_capability_space);
    assert(reinterpret_cast<uintptr_t>(cap) < arch::memory::end_of_capability_space);
    auto table_index  = ((reinterpret_cast<uintptr_t>(cap) & (sizeof(capability_table_t) - 1)) - offsetof(capability_table_t, slots)) / sizeof(capability_t);
    auto generation   = reinterpret_cast<capability_table_t*>(reinterpret_cast<uintptr_t>(cap) & ~(sizeof(capability_table_t) - 1))->generation;
    auto cap_space_id = (reinterpret_cast<uintptr_t>(cap) - arch::memory::begin_of_capability_space) / sizeof(capability_table_t);
    return capability_handle_t {
      .table_index         = table_index,
      .generation          = generation,
      .capability_space_id = cap_space_id,
    };
  }
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_H_
