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

#ifndef CAPRESE_ARCH_COMMON_MEMORY_PAGE_H_
#define CAPRESE_ARCH_COMMON_MEMORY_PAGE_H_

#ifdef CONFIG_ARCH_RISCV64
#include <caprese/arch/rv64/memory/page.h>
#else
#error "Unknown Arch"
#endif

#include <caprese/memory/address.h>

namespace caprese::arch::memory {
  bool map(caprese::memory::virtual_address_t  root_page_table,
           caprese::memory::virtual_address_t  virt_addr,
           caprese::memory::physical_address_t phys_addr,
           uintptr_t                           flags);
  bool unmap(caprese::memory::virtual_address_t root_page_table, caprese::memory::virtual_address_t virt_addr);
  void shallow_copy_mapping(caprese::memory::virtual_address_t dst_root_page_table,
                            caprese::memory::virtual_address_t src_root_page_table,
                            caprese::memory::virtual_address_t begin,
                            caprese::memory::virtual_address_t end);
} // namespace caprese::arch::memory

#endif // CAPRESE_ARCH_COMMON_MEMORY_PAGE_H_
