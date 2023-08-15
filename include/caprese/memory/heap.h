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

#include <caprese/arch/boot_info.h>
#include <caprese/memory/address.h>

namespace caprese::memory {
  [[nodiscard]] bool init_heap(const arch::boot_info_t* boot_info);

  [[nodiscard]] mapped_address_t allocate(size_t size, size_t align);
  void                           deallocate(mapped_address_t addr);

  [[nodiscard]] size_t num_remaining_pages();
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_HEAP_H_
