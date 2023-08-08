#ifndef CAPRESE_ARCH_RV64_MEMORY_H_
#define CAPRESE_ARCH_RV64_MEMORY_H_

#include <cstddef>
#include <cstdint>

namespace caprese::arch::inline rv64 {
  constexpr size_t PAGE_SIZE = 0x1000;
#if defined(CONFIG_MMU_SV39)
  constexpr size_t MAX_PAGE_SIZE = 0x40000000;
#elif defined(CONFIG_MMU_SV48)
  constexpr size_t MAX_PAGE_SIZE = 0x8000000000;
#endif

  struct page_flags_t {
    int readable  : 1;
    int writable  : 1;
    int executable: 1;
    int user      : 1;
  };

  bool map_page(uintptr_t root_page_table, uintptr_t virtual_address, uintptr_t physical_address, page_flags_t flags, bool allocate);
  bool unmap_page(uintptr_t root_page_table, uintptr_t virtual_address);
  bool is_mapped_page(uintptr_t root_page_table, uintptr_t virtual_address);

  bool shallow_map_page(uintptr_t root_page_table, uintptr_t virtual_address, uintptr_t physical_address);
  bool copy_shallow_page_mapping(uintptr_t dst_page_table, uintptr_t src_page_table, uintptr_t virtual_address);
  bool is_shallow_mapped_page(uintptr_t root_page_table, uintptr_t virtual_address);

  uintptr_t get_current_root_page_table();
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_MEMORY_H_
