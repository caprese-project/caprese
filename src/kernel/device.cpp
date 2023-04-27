#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <caprese/kernel/device.h>
#include <caprese/lib/align.h>
#include <caprese/lib/console.h>
#include <caprese/lib/endian.h>
#include <caprese/lib/string.h>

namespace caprese {
  namespace {
    struct fdt_header {
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

    struct fdt_reserve_entry {
      uint64_t address;
      uint64_t size;
    };

    template<typename T>
    inline T next(const char* begin, size_t& offset) {
      auto result = reinterpret_cast<const T*>(begin + offset);
      offset += sizeof(T);
      return *result;
    }

    inline const char* next_str(const char* begin, size_t& offset) {
      auto result = begin + offset;
      offset += strlen(result) + 1;
      return result;
    }

    enum struct structure_token : uint32_t {
      begin_node = swap_endian(0x01),
      end_node   = swap_endian(0x02),
      prop       = swap_endian(0x03),
      nop        = swap_endian(0x04),
      end        = swap_endian(0x09),
    };
  } // namespace

  error_t device_node::find_node_by_name(device_node& node, const char* name) const {
    log_assert("device_node", this->begin_node_address != nullptr && this->begin_string_block != nullptr, "Node is not initialized.");
    if (name == nullptr) {
      log_error("device_node", "name must not be null.");
      return error_t::invalid_argument;
    }

    size_t current = 0;
    int    depth   = 0;
    while (true) {
      auto token_address = this->begin_node_address + current;
      auto token         = next<structure_token>(this->begin_node_address, current);
      switch (token) {
        using enum structure_token;
        [[likely]] case prop: {
          auto                  len     = next<uint32_t>(this->begin_node_address, current);
          [[maybe_unused]] auto nameoff = next<uint32_t>(this->begin_node_address, current);
          current += swap_endian(len);
          break;
        }
        case begin_node: {
          ++depth;

          auto   node_name   = next_str(this->begin_node_address, current);
          size_t name_length = 0;
          if (auto at_pos = strchr(node_name, '@')) {
            name_length = at_pos - node_name;
          } else {
            name_length = strlen(node_name);
          }

          if (strlen(name) != name_length || strncmp(name, node_name, name_length) != 0) {
            break;
          }

          log_info("device", "Device '%s' found.", node_name);
          node.begin_node_address = token_address;
          node.begin_string_block = this->begin_string_block;
          return error_t::ok;
        }
        case end_node:
          if (--depth <= 0) {
            goto end;
          }
          break;
          [[unlikely]] case end: goto end;
          [[unlikely]] case nop: break;
          [[unlikely]] default: log_warn("device", "Unknown token: %p", token);
          break;
      }
      current = round_up(current, sizeof(uint32_t));
    }

  end:
    log_warn("device", "Device '%s' not found.", name);
    return error_t::not_found;
  }

  error_t device_node::find_property_by_name(device_node_property& property, const char* name) const {
    log_assert("device_node", this->begin_node_address != nullptr && this->begin_string_block != nullptr, "Node is not initialized.");
    if (name == nullptr) {
      log_error("device_node", "name must not be null.");
      return error_t::invalid_argument;
    }

    size_t current = 0;
    int    depth   = 0;
    while (true) {
      auto token = next<structure_token>(this->begin_node_address, current);
      switch (token) {
        using enum structure_token;
        [[likely]] case prop: {
          auto len     = next<uint32_t>(this->begin_node_address, current);
          auto nameoff = next<uint32_t>(this->begin_node_address, current);

          if (depth == 1) {
            auto property_name = this->begin_string_block + swap_endian(nameoff);
            if (strcmp(name, property_name) == 0) {
              property.name        = property_name;
              property.data        = this->begin_node_address + current;
              property.data_length = swap_endian(len);
              log_info("device", "Property '%s' found.", property_name);
              return error_t::ok;
            }
          }

          current += swap_endian(len);
          break;
        }
        case begin_node: {
          ++depth;
          [[maybe_unused]] auto node_name = next_str(this->begin_node_address, current);
          break;
        }
        case end_node:
          if (--depth <= 0) {
            goto end;
          }
          break;
          [[unlikely]] case end: goto end;
          [[unlikely]] case nop: break;
          [[unlikely]] default: log_warn("device", "Unknown token: %p", token);
          break;
      }
      current = round_up(current, sizeof(uint32_t));
    }

  end:
    log_warn("device", "Property '%s' not found.", name);
    return error_t::not_found;
  }

  error_t device::get_root_node(device_node& node) const {
    log_assert("device", this->dtb != nullptr, "Device is not initialized.");

    auto header = reinterpret_cast<const fdt_header*>(this->dtb);

    node.begin_node_address = dtb + swap_endian(header->off_dt_struct);
    node.begin_string_block = dtb + swap_endian(header->off_dt_strings);

    return error_t::ok;
  }

  error_t device::from_dtb(device& device, const char* dtb) {
    device.dtb = nullptr;

    if (dtb == nullptr) {
      log_error("device", "dtb must not be null.");
      return error_t::invalid_argument;
    }

    auto header = reinterpret_cast<const fdt_header*>(dtb);

    log_info("device", "Magic number:                %p", swap_endian(header->magic));
    log_info("device", "Total DTB size:              %p", swap_endian(header->totalsize));
    log_info("device", "DTB version:                 %x", swap_endian(header->version));
    log_info("device", "DTB last compatible version: %x", swap_endian(header->last_comp_version));
    log_info("device", "Boot phys cpuid:             %d", swap_endian(header->boot_cpuid_phys));
    log_info("device", "Size of structure block:     %p", swap_endian(header->size_dt_struct));
    log_info("device", "Size of string block:        %p", swap_endian(header->size_dt_strings));

    if (header->magic != swap_endian(0xD00DFEED)) {
      log_error("device", "The magic number was unexpected.");
      return error_t::invalid_argument;
    }

    device.dtb = dtb;

    return error_t::ok;

    // auto reserve_entry = reinterpret_cast<const fdt_reserve_entry*>(dtb + swap_endian(header->off_mem_rsvmap));
    // while (true) {
    //   if (reserve_entry->address == 0 && reserve_entry->size == 0) {
    //     break;
    //   }
    //   log_info("device::from_dtb", "Memory reservation block found. address: %p, size: %p", swap_endian(reserve_entry->address),
    //   swap_endian(reserve_entry->size));
    //   ++reserve_entry;
    // }
  }
} // namespace caprese
