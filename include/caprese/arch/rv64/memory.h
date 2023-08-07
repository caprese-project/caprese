#ifndef CAPRESE_ARCH_RV64_MEMORY_H_
#define CAPRESE_ARCH_RV64_MEMORY_H_

#include <cstddef>
#include <cstdint>

namespace caprese::arch::inline rv64 {
  constexpr size_t PAGE_SIZE = 0x1000;

  struct page_flags_t {
    int readable  : 1;
    int writable  : 1;
    int executable: 1;
    int user      : 1;
  };

  bool map_page(uintptr_t root_page_table, uintptr_t virtual_address, uintptr_t physical_address, page_flags_t flags, bool allocate);
  bool unmap_page(uintptr_t root_page_table, uintptr_t virtual_address);

  bool is_mapped_page(uintptr_t root_page_table, uintptr_t virtual_address);

  uintptr_t get_kernel_root_page_table();
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_MEMORY_H_
