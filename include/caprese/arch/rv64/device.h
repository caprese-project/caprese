#ifndef CAPRESE_ARCH_RV64_DEVICE_H_
#define CAPRESE_ARCH_RV64_DEVICE_H_

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <tuple>
#include <utility>

#include <caprese/arch/boot_info.h>
#include <caprese/util/align.h>
#include <caprese/util/lambda.h>
#include <caprese/util/panic.h>

namespace caprese::arch::inline rv64 {
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

  constexpr uint32_t FDT_HEADER_MAGIC = 0xd00dfeed;
  constexpr uint32_t FDT_BEGIN_NODE   = 0x00000001;
  constexpr uint32_t FDT_END_NODE     = 0x00000002;
  constexpr uint32_t FDT_PROP         = 0x00000003;
  constexpr uint32_t FDT_NOP          = 0x00000004;
  constexpr uint32_t FDT_END          = 0x00000009;

  struct memory_flags_t {
    int device     : 1;
    int readable   : 1;
    int writable   : 1;
    int unavailable: 1;
  };

  struct scan_callback_args_t {
    memory_flags_t flags;
    uintptr_t      address;
    size_t         size;
    const char*    device_name;
  };

  template<typename T>
  concept scan_callback = requires(T f) {
    { f(static_cast<scan_callback_args_t*>(nullptr)) } -> std::convertible_to<void>;
  };

  template<scan_callback F>
  void scan_device(const boot_info_t* boot_info, F callback) {
    const fdt_header_t* header = reinterpret_cast<const fdt_header_t*>(boot_info->device_tree_blob);
    if (header->magic != std::byteswap(FDT_HEADER_MAGIC)) {
      panic("Invalid device tree header magic: 0x%x", header->magic);
    }

    const char* struct_block = boot_info->device_tree_blob + std::byteswap(header->off_dt_struct);
    const char* string_block = boot_info->device_tree_blob + std::byteswap(header->off_dt_strings);

    const size_t struct_block_size = std::byteswap(header->size_dt_struct);

    const char* current = struct_block;

    const auto to_u32 = [](const char* ptr) {
      return std::byteswap(*reinterpret_cast<const uint32_t*>(ptr));
    };

    const auto next_u32 = [&current, &to_u32]() {
      uint32_t result = to_u32(current);
      current += sizeof(uint32_t);
      return result;
    };

    const auto next_str = [&current]() {
      const char* str = current;
      current += strlen(str) + 1;
      return str;
    };

    const auto align = [&current]() {
      current = reinterpret_cast<const char*>(round_up(reinterpret_cast<uintptr_t>(current), sizeof(uint32_t)));
    };

    const auto scan = make_recursive_lambda([&](auto self, const char* node_name, size_t address_cells, size_t size_cells) -> bool {
      align();

      size_t child_address_cells = 2;
      size_t child_size_cells    = 1;

      while (current < struct_block + struct_block_size) {
        uint32_t token = next_u32();
        switch (token) {
          case FDT_BEGIN_NODE:
            if (self(next_str(), child_address_cells, child_size_cells)) [[unlikely]] {
              return true;
            }
            break;
          case FDT_END_NODE:
            align();
            return false;
          case FDT_PROP: {
            uint32_t len     = next_u32();
            uint32_t nameoff = next_u32();

            const char* prop_name = string_block + nameoff;
            const char* prop_data = current;
            current += len;

            if (strcmp(prop_name, "#address-cells") == 0) {
              child_address_cells = to_u32(prop_data);
            } else if (strcmp(prop_name, "#size-cells") == 0) {
              child_size_cells = to_u32(prop_data);
            } else if (strcmp(prop_name, "reg") == 0) {
              for (size_t i = 0; i < len; i += (address_cells + size_cells) * sizeof(uint32_t)) {
                uintptr_t address = 0;
                for (size_t j = 0; j < address_cells; ++j) {
                  address <<= 32;
                  address |= to_u32(prop_data + i + j * sizeof(uint32_t));
                }

                size_t size = 0;
                for (size_t j = 0; j < size_cells; ++j) {
                  size <<= 32;
                  size |= to_u32(prop_data + i + address_cells * sizeof(uint32_t) + j * sizeof(uint32_t));
                }

                scan_callback_args_t args {};

                args.device_name = node_name;
                args.address     = address;
                args.size        = size;

                if (strncmp(node_name, "memory", 6) == 0) {
                  args.flags.device      = 0;
                  args.flags.readable    = 1;
                  args.flags.writable    = 1;
                  args.flags.unavailable = 0;
                } else if (strncmp(node_name, "mmode_resv", 10) == 0) {
                  args.flags.device      = 0;
                  args.flags.readable    = 0;
                  args.flags.writable    = 0;
                  args.flags.unavailable = 1;
                } else {
                  args.flags.device      = 1;
                  args.flags.readable    = 1;
                  args.flags.writable    = 1;
                  args.flags.unavailable = 0;
                }

                callback(&args);
              }
            }
            break;
          }
          case FDT_NOP:
            break;
          case FDT_END:
            return true;
          default:
            printf("Unknown token: 0x%x\n", token);
        }

        align();
      }

      return true;
    });

    scan(nullptr, 2, 1);
  }

  [[nodiscard]] std::pair<uintptr_t, size_t> get_mmode_rsrv_space(const boot_info_t* boot_info);
  [[nodiscard]] std::pair<uintptr_t, size_t> get_dtb_space(const boot_info_t* boot_info);
  [[nodiscard]] std::pair<uintptr_t, size_t> get_kernel_space();
  [[nodiscard]] std::pair<uintptr_t, size_t> get_payload_space();

  // 0: m-mode, 1: dtb, 2: kernel, 3: payload
  using reserved_ram_space_t = std::tuple<std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>, std::pair<uintptr_t, size_t>>;
  [[nodiscard]] reserved_ram_space_t get_reserved_ram_spaces(const boot_info_t* boot_info);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_DEVICE_H_
