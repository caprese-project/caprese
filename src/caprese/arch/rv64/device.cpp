#include <algorithm>

#include <caprese/arch/memory.h>
#include <caprese/arch/rv64/device.h>
#include <caprese/memory/address.h>
#include <caprese/util/align.h>

extern "C" {
  extern const char _kernel_start[];
  extern const char _kernel_end[];
  extern const char _payload_start[];
  extern const char _payload_end[];
}

namespace caprese::arch::inline rv64 {
  std::tuple<std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>>
      get_reserved_ram_spaces(const boot_info_t* boot_info) {
    std::tuple<std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>> result;

    uintptr_t mmode_resv_start = -1;
    uintptr_t mmode_resv_end   = 0;
    scan_device(boot_info, [&mmode_resv_start, &mmode_resv_end](scan_callback_args_t* args) {
      if (strncmp(args->device_name, "mmode_resv", 10) == 0) {
        mmode_resv_start = std::min(mmode_resv_start, args->address);
        mmode_resv_end   = std::max(mmode_resv_end, args->address + args->size);
      }
    });
    std::get<0>(result) = {
      mmode_resv_start,
      round_up(mmode_resv_end - mmode_resv_start, PAGE_SIZE),
    };

    const fdt_header_t* header = reinterpret_cast<const fdt_header_t*>(boot_info->device_tree_blob);
    if (header->magic != std::byteswap(FDT_HEADER_MAGIC)) {
      panic("Invalid device tree header magic: 0x%x", header->magic);
    }
    std::get<1>(result) = {
      memory::mapped_address_t::from(header).physical_address().value,
      round_up(std::byteswap(header->totalsize), PAGE_SIZE),
    };

    std::get<2>(result) = {
      memory::mapped_address_t::from(_kernel_start).physical_address().value,
      round_up(_kernel_end - _kernel_start, PAGE_SIZE),
    };

    std::get<3>(result) = {
      memory::mapped_address_t::from(_payload_start).physical_address().value,
      round_up(_payload_end - _payload_start, PAGE_SIZE),
    };

    return result;
  }
} // namespace caprese::arch::inline rv64
