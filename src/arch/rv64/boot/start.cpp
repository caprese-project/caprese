#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <utility>

#include <kernel/arch/csr.h>

constexpr const char* tag = "boot/start";

struct page_table_entry_t {
  uint64_t v  : 1;
  uint64_t r  : 1;
  uint64_t w  : 1;
  uint64_t x  : 1;
  uint64_t u  : 1;
  uint64_t g  : 1;
  uint64_t a  : 1;
  uint64_t d  : 1;
  uint64_t rsv: 2;
  uint64_t npn: 44;
};

alignas(0x1000) page_table_entry_t root_page_table[0x1000 / sizeof(page_table_entry_t)];

#if defined(CONFIG_MMU_SV39)
constexpr size_t PAGE_SIZE_BIT = 9 + 9 + 12;
#elif defined(CONFIG_MMU_SV48)
constexpr size_t PAGE_SIZE_BIT = 9 + 9 + 9 + 12;
#endif

constexpr size_t PAGE_SIZE = 1 << PAGE_SIZE_BIT;

extern const char _payload_start[];

void map_mapped_space(void) {
  page_table_entry_t* page_table = root_page_table;

  for (uintptr_t phys = 0; phys < CONFIG_MAPPED_SPACE_SIZE; phys += PAGE_SIZE) {
    uintptr_t virt  = CONFIG_MAPPED_SPACE_BASE + phys;
    size_t    index = (virt >> PAGE_SIZE_BIT) & 0x1ff;

    page_table[index] = {
      .v   = 1,
      .r   = 1,
      .w   = 1,
      .x   = 1,
      .u   = 0,
      .g   = 1,
      .a   = 1,
      .d   = 1,
      .rsv = 0,
      .npn = phys >> 12,
    };

    printf("Mapping %p -> %p\n", phys, virt);
  }
}

extern "C" [[noreturn]] void start(uintptr_t hartid, uintptr_t dtb) {
  printf("\nBooting on Hart %ld...\n", hartid);
  printf("Device Tree Blob:               %p\n", reinterpret_cast<void*>(dtb));
  printf("Payload Start Physical Address: %p\n", _payload_start);
  printf("Payload Start Virtual Address:  %p\n", CONFIG_MAPPED_SPACE_BASE + _payload_start);

  printf("Mapping kernel spaces...\n");
  map_mapped_space();
  printf("Kernel spaces mapped.\n");

  uintptr_t satp = reinterpret_cast<uintptr_t>(root_page_table) >> 12;
#if defined(CONFIG_MMU_SV39)
  satp |= SATP_MODE_SV39;
#elif defined(CONFIG_MMU_SV48)
  satp |= SATP_MODE_SV48;
#endif

  // When MMU is enabled, a page fault occurs and jumps to the kernel entry point (stvec address).

  asm volatile("csrw stvec, %0" : : "r"(CONFIG_MAPPED_SPACE_BASE + _payload_start));

  printf("Jumping to Kernel...\n");

  asm volatile("mv a0, %0" : : "r"(hartid));
  asm volatile("mv a1, %0" : : "r"(CONFIG_MAPPED_SPACE_BASE + dtb));

  asm volatile("sfence.vma zero, zero");
  asm volatile("csrw satp, %0" : : "r"(satp));
  asm volatile("sfence.vma zero, zero");

  std::unreachable();
}
