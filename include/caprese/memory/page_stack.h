/**
 * @file page_stack.h
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

#ifndef CAPRESE_MEMORY_PAGE_STACK_H_
#define CAPRESE_MEMORY_PAGE_STACK_H_

#include <caprese/capability/capability.h>

namespace caprese::memory {
  void                            push_page(capability::capability_handle_t memory_cap);
  capability::capability_handle_t pop_page();
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_PAGE_STACK_H_
