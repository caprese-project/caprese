#ifndef ARCH_RV64_KERNEL_ARCH_DTB_H_
#define ARCH_RV64_KERNEL_ARCH_DTB_H_

#include <cstdint>

#include <kernel/address.h>

struct fdt_header_t {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};

struct fdt_reserve_entry_t {
  uint64_t address;
  uint64_t size;
};

constexpr uint32_t FDT_HEADER_MAGIC = 0xd00dfeed;
constexpr uint32_t FDT_BEGIN_NODE   = 0x00000001;
constexpr uint32_t FDT_END_NODE     = 0x00000002;
constexpr uint32_t FDT_PROP         = 0x00000003;
constexpr uint32_t FDT_NOP          = 0x00000004;
constexpr uint32_t FDT_END          = 0x00000009;

// clang-format off

constexpr const char* FDT_U32_TYPES[] = {
  "#address-cells",
  "#interrupt-cells",
  "#size-cells",
  "cache-block-size",
  "cache-line-size",
  "cache-op-block-size",
  "cache-sets",
  "cache-size",
  "d-cache-block-size",
  "d-cache-line-size",
  "d-cache-sets",
  "d-cache-size",
  "d-tlb-sets",
  "d-tlb-sets",
  "d-tlb-size",
  "i-cache-block-size",
  "i-cache-line-size",
  "i-cache-sets",
  "i-cache-size",
  "i-tlb-sets",
  "reservation-granule-size",
  "riscv,ndev",
  "tlb-sets",
  "tlb-size",
  "virtual-reg",
};

constexpr const char* FDT_U64_TYPES[] = {
  "cpu-release-addr",
};

constexpr const char* FDT_STR_TYPES[] = {
  "bootargs",
  "device_type",
  "mmu-type",
  "model",
  "name",
  "riscv,isa",
  "status",
  "stdin-path",
  "stdout-path",
};

constexpr const char* FDT_ARRAY_TYPES[] = {
  "clock-frequency",
  "dma-ranges",
  "initial-mapped-area",
  "interrupt-map-mask",
  "interrupt-map",
  "interrupts-extended",
  "interrupts",
  "ranges",
  "reg",
  "timebase-frequency",
};

constexpr const char* FDT_PHANDLE_TYPES[] = {
  "interrupt-parent",
  "next-level-cache",
  "phandle",
};

constexpr const char* FDT_STR_LIST_TYPES[] = {
  "compatible",
  "enable-method",
};

// clang-format on

struct dtb_node_t {
  const char*         _next;
  uintptr_t           unit_address;
  const fdt_header_t* header;
  uint32_t            address_cells;
  uint32_t            size_cells;
  char                name[32];
};

enum struct dtb_prop_type_t {
  empty,
  u32,
  u64,
  str,
  array,
  phandle,
  str_list,
  unknown,
};

struct dtb_prop_t {
  const char*     name;
  dtb_prop_type_t type;

  union {
    uint32_t    u32;
    uint64_t    u64;
    const char* str;

    struct {
      uint32_t       length;
      const uint8_t* data;
    } array;

    uint32_t phandle;

    struct {
      uint32_t    length;
      const char* data;
    } str_list;
  };
};

dtb_prop_type_t find_prop_type(const char* name);

void for_each_dtb_node(map_addr_t dtb, bool (*callback)(const dtb_node_t*));
void for_each_dtb_prop(const dtb_node_t* node, bool (*callback)(const dtb_node_t*, const dtb_prop_t*));

#endif // ARCH_RV64_KERNEL_ARCH_DTB_H_
