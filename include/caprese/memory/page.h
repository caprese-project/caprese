/**
 * @file page.h
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

#ifndef CAPRESE_MEMORY_PAGE_H_
#define CAPRESE_MEMORY_PAGE_H_

#include <caprese/arch/memory.h>
#include <caprese/memory/address.h>

namespace caprese::memory {
  bool map(mapped_address_t root_page_table, virtual_address_t virtual_address, physical_address_t physical_address, arch::page_flags_t flags, bool allocate);
  bool unmap(mapped_address_t root_page_table, virtual_address_t virtual_address);
  bool is_mapped(mapped_address_t root_page_table, virtual_address_t virtual_address);

  bool shallow_map(mapped_address_t root_page_table, virtual_address_t virtual_address, physical_address_t physical_address);
  bool copy_shallow_mapping(mapped_address_t dst_page_table, mapped_address_t src_page_table, virtual_address_t virtual_address);
  bool is_shallow_mapped(mapped_address_t root_page_table, virtual_address_t virtual_address);

  mapped_address_t get_current_root_page_table();
  mapped_address_t get_kernel_root_page_table();
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_PAGE_H_
