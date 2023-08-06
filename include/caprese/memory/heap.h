/**
 * @file heap.h
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

#ifndef CAPRESE_MEMORY_HEAP_H_
#define CAPRESE_MEMORY_HEAP_H_

#include <cstddef>

#include <caprese/capability/capability.h>
#include <caprese/memory/address.h>

namespace caprese::memory {
  struct heap_block_t;

  struct heap_header_t {
    capability::capability_handle_t this_cap;
    heap_block_t*                   prev;
    heap_block_t*                   next;
    uint16_t                        used;
    uint16_t                        released;
  };

  struct heap_block_t {
    heap_header_t header;
    uint8_t       data[page_size() - sizeof(heap_header_t)];
  };

  static_assert(sizeof(heap_block_t) == page_size());

  constexpr size_t max_heap_alloc_size() {
    return sizeof(heap_block_t::data);
  }

  void append_heap(capability::capability_handle_t memory_cap_handle);

  virtual_address_t allocate(size_t size);
  void              deallocate(virtual_address_t addr, size_t size);
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_HEAP_H_
