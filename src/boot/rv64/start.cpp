/**
 * @file start.cpp
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

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <caprese/arch/rv64/csr.h>
#include <caprese/arch/rv64/sbi.h>

extern "C" {
  extern const char _boot_loader_start[];
  extern const char _boot_loader_end[];
  extern const char _payload_base[];
  extern char       _page_table[];
}

namespace boot {
  using namespace caprese::arch;

  struct page_table_entry_t {
    uint64_t v               : 1;
    uint64_t r               : 1;
    uint64_t w               : 1;
    uint64_t x               : 1;
    uint64_t u               : 1;
    uint64_t g               : 1;
    uint64_t a               : 1;
    uint64_t d               : 1;
    uint64_t rsv             : 2;
    uint64_t next_page_number: 44;
  };

  [[noreturn]] void panic(const char* msg) {
    sbi_debug_console_write(strlen(msg), reinterpret_cast<uintptr_t>(msg), 0);

    while (true) {
      sbi_hart_stop();
    }
  }

  void map_kernel_window() {
#if defined(CONFIG_MMU_SV39)
    constexpr size_t page_size_bit = 9 + 9 + 12;
#elif defined(CONFIG_MMU_SV48)
    constexpr size_t page_size_bit = 9 + 9 + 9 + 12;
#endif

    constexpr size_t    page_size  = 1 << page_size_bit;
    page_table_entry_t* page_table = reinterpret_cast<page_table_entry_t*>(_page_table);

    for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += page_size) {
      uintptr_t virt                     = CONFIG_MAPPED_SPACE_BASE + phys;
      size_t    index                    = (virt >> page_size_bit) & 0x1ff;
      page_table[index].v                = 1;
      page_table[index].r                = 1;
      page_table[index].w                = 1;
      page_table[index].x                = 1;
      page_table[index].g                = 1;
      page_table[index].next_page_number = phys >> 12;
      printf("Mapped page: virtual address=0x%016lx, physical address=0x%016lx, size=0x%lx\n", virt, phys, page_size);
    }

    for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += page_size) {
      uintptr_t virt                     = phys;
      size_t    index                    = (virt >> page_size_bit) & 0x1ff;
      page_table[index].v                = 1;
      page_table[index].r                = 1;
      page_table[index].w                = 1;
      page_table[index].x                = 1;
      page_table[index].g                = 1;
      page_table[index].next_page_number = phys >> 12;
      printf("Mapped page: virtual address=0x%016lx, physical address=0x%016lx, size=0x%lx\n", virt, phys, page_size);
    }
  }

  void enable_mmu() {
    uintptr_t satp = reinterpret_cast<uintptr_t>(_page_table) >> 12;
#if defined(CONFIG_MMU_SV39)
    satp |= caprese::arch::SATP_MODE_SV39;
    printf("Enabling MMU in sv39 mode...\n");
#elif defined(CONFIG_MMU_SV48)
    satp |= caprese::arch::SATP_MODE_SV48;
    printf("Enabling MMU in sv48 mode...\n");
#endif

    asm volatile("sfence.vma zero, zero");
    asm volatile("csrw satp, %0" : : "r"(satp));
    asm volatile("sfence.vma zero, zero");
  }

  [[noreturn]] void jump_to_kernel(uint64_t hartid, const char* device_tree_blob) {
    asm volatile("mv a0, %0" : : "r"(hartid));
    asm volatile("mv a1, %0" : : "r"(CONFIG_MAPPED_SPACE_BASE + device_tree_blob));

    uintptr_t kernel_entry = CONFIG_MAPPED_SPACE_BASE + reinterpret_cast<uintptr_t>(_payload_base);
    asm volatile("jr %0" : : "r"(kernel_entry));

    panic("UNREACHABLE\n");
  }
} // namespace boot

extern "C" [[noreturn]] void start(uint64_t hartid, const char* device_tree_blob) {
  using namespace boot;

  printf("\nBooting on hart %lu...\n\n", hartid);

  printf("Initializing page tables...\n");
  map_kernel_window();
  printf("Page table initialization completed.\n\n");

  printf("Enabling MMU...\n");
  enable_mmu();
  printf("MMU enabled.\n\n");

  printf("Jumping to kernel...\n\n");
  jump_to_kernel(hartid, device_tree_blob);
}
