#ifndef CAPRESE_ARCH_RV64_MEMORY_H_
#define CAPRESE_ARCH_RV64_MEMORY_H_

#include <cstddef>
#include <cstdint>

namespace caprese::arch::inline rv64 {
  constexpr size_t PAGE_SIZE_BIT = 12;
  constexpr size_t PAGE_SIZE     = 1 << PAGE_SIZE_BIT;

  struct page_flags_t {
    int readable  : 1;
    int writable  : 1;
    int executable: 1;
    int user      : 1;
  };

  bool map_page(uintptr_t root_page_table, uintptr_t virtual_address, uintptr_t physical_address, page_flags_t flags, bool allocate);
  bool unmap_page(uintptr_t root_page_table, uintptr_t virtual_address);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_MEMORY_H_
