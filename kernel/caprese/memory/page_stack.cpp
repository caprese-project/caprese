/**
 * @file page_stack.cpp
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

#include <caprese/memory/heap.h>
#include <caprese/memory/page_stack.h>
#include <caprese/panic.h>

#include <libcaprese/util/array_size.h>

namespace caprese::memory {
  namespace {
    struct stack_block_t;

    struct stack_header_t {
      stack_block_t* prev;
      size_t         pos;
    };

    struct stack_block_t {
      stack_header_t                  header;
      capability::capability_handle_t caps[(max_heap_alloc_size() - sizeof(stack_header_t)) / sizeof(capability::capability_handle_t)];
    };

    static_assert(sizeof(stack_block_t) == max_heap_alloc_size());

    stack_block_t* tail;
  } // namespace

  void push_page(capability::capability_handle_t memory_cap) {
    if (tail == nullptr) [[unlikely]] {
      tail              = allocate(sizeof(stack_block_t)).as<stack_block_t*>();
      tail->header.prev = nullptr;
      tail->header.pos  = 0;
    }
    if (tail->header.pos == libcaprese::util::array_size_of(tail->caps)) {
      auto new_tail         = allocate(sizeof(stack_block_t)).as<stack_block_t*>();
      new_tail->header.prev = tail;
      new_tail->header.pos  = 0;
      tail                  = new_tail;
    }
    tail->caps[tail->header.pos] = memory_cap;
    ++tail->header.pos;
  }

  capability::capability_handle_t pop_page() {
    if (tail == nullptr) [[unlikely]] {
      panic("Stack is empty.");
    }

    if (tail->header.pos == 0) [[unlikely]] {
      auto old_tail = tail;
      tail          = tail->header.prev;
      deallocate(old_tail, sizeof(tail));
      return pop_page();
    }

    --tail->header.pos;
    return tail->caps[tail->header.pos];
  }
} // namespace caprese::memory
