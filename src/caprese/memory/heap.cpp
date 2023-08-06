/**
 * @file heap.cpp
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

#include <caprese/memory/heap.h>
#include <caprese/memory/page_stack.h>
#include <caprese/panic.h>

namespace caprese::memory {
  namespace {
    // in_use_list[i]: list of blocks with free space between 2^i and 2^(i+1)-1 (byte)
    // in_use_list[0]: list of empty blocks
    heap_block_t* in_use_list[12];
  } // namespace

  void append_heap(capability::capability_handle_t memory_cap_handle) {
    auto& cap = capability::get_capability(memory_cap_handle);
    assert(cap.valid() && cap.type() == capability::capability_type::memory && cap.memory.device == 0);

    if (in_use_list[0] == nullptr) [[unlikely]] {
      in_use_list[0]              = cap.memory.address().as<heap_block_t*>();
      in_use_list[0]->header.prev = nullptr;
    } else {
      in_use_list[0]->header.next              = cap.memory.address().as<heap_block_t*>();
      in_use_list[0]->header.next->header.prev = in_use_list[0];
      in_use_list[0]                           = in_use_list[0]->header.next;
    }

    in_use_list[0]->header.this_cap = memory_cap_handle;
    in_use_list[0]->header.next     = nullptr;
    in_use_list[0]->header.used     = 0;
    in_use_list[0]->header.released = 0;
  }

  virtual_address_t allocate(size_t size) {
    assert(size > 0);
    assert(size <= max_heap_alloc_size());

    unsigned n = 0;
    for (unsigned i = 0; i < page_size_bit(); ++i) {
      if (size & (1 << i)) {
        n = i;
      }
    }

    virtual_address_t result = static_cast<uintptr_t>(0);

    do {
      if (in_use_list[n] != nullptr) {
        assert(max_heap_alloc_size() - in_use_list[n]->header.used >= size);
        result = reinterpret_cast<uintptr_t>(in_use_list[n]->data + in_use_list[n]->header.used);
        break;
      }
    } while (++n < page_size_bit());

    if (result.value() == 0) [[unlikely]] {
      if (in_use_list[0] == nullptr) [[unlikely]] {
        append_heap(pop_page());
      }
      n      = 0;
      result = reinterpret_cast<uintptr_t>(in_use_list[0]->data);
    }

    in_use_list[n]->header.used += size;
    auto     rem = max_heap_alloc_size() - in_use_list[n]->header.used;
    unsigned m   = 0;
    for (unsigned i = 0; i < page_size_bit(); ++i) {
      if (rem & (1 << i)) {
        m = i;
      }
    }

    if (m == 0) [[unlikely]] {
      in_use_list[n] = in_use_list[n]->header.prev;
      if (in_use_list[n] != nullptr) {
        in_use_list[n]->header.next = nullptr;
      }
    } else if (n != m) {
      if (in_use_list[m] == nullptr) [[unlikely]] {
        in_use_list[m]              = in_use_list[n];
        in_use_list[n]              = in_use_list[n]->header.prev;
        in_use_list[n]->header.next = nullptr;
        in_use_list[m]->header.prev = nullptr;
        in_use_list[m]->header.next = nullptr;
      } else {
        in_use_list[n]                           = in_use_list[n]->header.prev;
        in_use_list[m]->header.next              = in_use_list[n]->header.next;
        in_use_list[n]->header.next              = nullptr;
        in_use_list[m]->header.next->header.prev = in_use_list[m]->header.next;
        in_use_list[m]                           = in_use_list[m]->header.next;
      }
    }

    return result;
  }

  void deallocate(virtual_address_t addr, size_t size) {
    auto block = reinterpret_cast<heap_block_t*>(addr.value() & ~page_mask());
    block->header.released += size;

    assert(block->header.used >= block->header.released);

    if (block->header.used == block->header.released) [[unlikely]] {
      auto prev = block->header.prev;
      auto next = block->header.next;
      if (prev != nullptr) {
        prev->header.next = next;
      }
      if (next != nullptr) {
        next->header.prev = prev;
      }
      append_heap(block->header.this_cap);
    }
  }
} // namespace caprese::memory
