#include <bit>
#include <cstring>
#include <iterator>
#include <utility>

#include <kernel/arch/dtb.h>
#include <kernel/setup.h>
#include <log/log.h>

extern "C" {
  extern const char _kernel_start[];
  extern const char _kernel_end[];
  extern const char _payload_start[];
  extern const char _payload_end[];
}

namespace {
  constexpr const char* tag = "kernel/setup";

  struct region_t {
    phys_addr_t start;
    phys_addr_t end;
  };

  // a - b
  inline void subtract_region(region_t a, region_t b, region_t (&dst)[2]) {
    dst[0] = { .start = phys_addr_t::from(nullptr), .end = phys_addr_t::from(nullptr) };
    dst[1] = { .start = phys_addr_t::from(nullptr), .end = phys_addr_t::from(nullptr) };

    if (a.start < b.start) {
      // +-----+---+-----+---+-----+
      // | ... | a | ... | b | ... |
      // +-----+---+-----+---+-----+
      if (a.end <= b.start) {
        dst[0] = a;
      }
      // +-----+---+---------+---+-----+
      // | ... | a | a and b | b | ... |
      // +-----+---+---------+---+-----+
      else if (a.end <= b.end) {
        dst[0].start = a.start;
        dst[0].end   = b.start;
      }
      // +-----+---+---------+---+-----+
      // | ... | a | a and b | a | ... |
      // +-----+---+---------+---+-----+
      else {
        dst[0].start = a.start;
        dst[0].end   = b.start;
        dst[1].start = b.end;
        dst[1].end   = a.end;
      }
    } else if (b.end < a.end) {
      // +-----+---+-----+---+-----+
      // | ... | b | ... | a | ... |
      // +-----+---+-----+---+-----+
      if (b.end <= a.start) {
        dst[0] = a;
      }
      // +-----+---+---------+---+-----+
      // | ... | b | a and b | a | ... |
      // +-----+---+---------+---+-----+
      else {
        dst[0].start = b.end;
        dst[0].end   = a.end;
      }
    }
  }

  __init_data region_t reserved_region[CONFIG_MAX_RESERVED_REGIONS];

  __init_data size_t reserved_region_count;

  __init_data region_t memory_region[CONFIG_MAX_MEMORY_REGIONS];

  __init_data size_t memory_region_count;

  __init_data region_t device_region[CONFIG_MAX_DEVICE_REGIONS];

  __init_data size_t device_region_count;

  __init_code void dump_devices(map_addr_t dtb) {
    for_each_dtb_node(dtb, [](const dtb_node_t* node) {
      logd(tag, "Device found: %s (%p)", node->name, node->unit_address);

      for_each_dtb_prop(node, []([[maybe_unused]] const dtb_node_t*, const dtb_prop_t* prop) {
        switch (prop->type) {
          using enum dtb_prop_type_t;
          case empty:
            logd(tag, "  Property found: %s (empty)", prop->name);
            break;
          case u32:
            logd(tag, "  Property found: %s %u (u32)", prop->name, prop->u32);
            break;
          case u64:
            logd(tag, "  Property found: %s %lu (u64)", prop->name, prop->u64);
            break;
          case str:
            logd(tag, "  Property found: %s %s (str)", prop->name, prop->str);
            break;
          case array:
            logd(tag, "  Property found: %s #%ubytes data (array)", prop->name, prop->array.length);
            break;
          case phandle:
            logd(tag, "  Property found: %s %u (phandle)", prop->name, prop->phandle);
            break;
          case str_list:
            logd(tag, "  Property found: %s (str_list)", prop->name, prop->str_list);
            for (uint32_t i = 0, offset = 0; offset < prop->str_list.length; offset += strlen(prop->str_list.data + offset) + 1) {
              logd(tag, "    [%02d]: %s", i++, prop->str_list.data + offset);
            }
            break;
          case unknown:
            logd(tag, "  Property found: %s (unknown)", prop->name);
            break;
        }

        return true;
      });

      return true;
    });
  }

  __init_code void for_each_reg(const dtb_node_t* node, const dtb_prop_t* prop, void (*callback)(region_t)) {
    assert(node != nullptr);
    assert(prop != nullptr);
    assert(callback != nullptr);
    assert(strcmp(prop->name, "reg") == 0);

    const uint32_t* data       = reinterpret_cast<const uint32_t*>(prop->array.data);
    const uint32_t  length     = prop->array.length / sizeof(uint32_t);
    const uint32_t  block_size = node->address_cells + node->size_cells;

    for (uint32_t i = 0; i < length; i += block_size) {
      uintptr_t base = 0;
      size_t    size = 0;

      for (size_t j = 0; j < node->address_cells; ++j) {
        base <<= 32;
        base |= std::byteswap(data[i + j]);
      }

      for (size_t j = 0; j < node->size_cells; ++j) {
        size <<= 32;
        size |= std::byteswap(data[i + node->address_cells + j]);
      }

      region_t region {
        .start = phys_addr_t::from(base),
        .end   = phys_addr_t::from(base + size),
      };

      callback(region);
    }
  }

  __init_code void push_reserved_region(const region_t region) {
    if (reserved_region_count >= std::size(reserved_region)) [[unlikely]] {
      panic("Too many reserved regions");
    }

    logd(tag, "Reserved region: %p - %p", region.start, region.end);
    reserved_region[reserved_region_count++] = region;
  }

  __init_code void push_memory_region(const region_t region) {
    if (memory_region_count >= std::size(memory_region)) [[unlikely]] {
      panic("Too many memory regions");
    }

    logd(tag, "Memory region: %p - %p", region.start, region.end);
    memory_region[memory_region_count++] = region;
  }

  __init_code void push_device_region(const region_t region) {
    if (device_region_count >= std::size(device_region)) [[unlikely]] {
      panic("Too many device regions");
    }

    logd(tag, "Device region: %p - %p", region.start, region.end);
    device_region[device_region_count++] = region;
  }

  __init_code void insert_memory_region_caps(task_t* root_task, region_t region, int flags, size_t index = 0) {
    if (region.start >= region.end) [[unlikely]] {
      return;
    }

    if (index < reserved_region_count) {
      region_t dst[2];
      subtract_region(region, reserved_region[index], dst);
      for (region_t reg : dst) {
        if (reg.start < reg.end) {
          insert_memory_region_caps(root_task, reg, flags, index + 1);
        }
      }

      return;
    }

    uint16_t size_bit = 0;

    while (region.start < region.end) {
      size_t size = region.end.as<uintptr_t>() - region.start.as<uintptr_t>();

      if (region.start.as<uintptr_t>() & (1ull << size_bit)) {
        if (size >= (1ull << size_bit)) {
          cap_t cap = make_memory_cap(flags, size_bit, region.start);
          logd(tag, "Memory capability created. addr=%p, size=%p(2^%-2d), type=%s", region.start, 1ull << size_bit, size_bit, flags & MEMORY_CAP_DEVICE ? "device" : "memory");

          if (!insert_cap(root_task, cap)) [[unlikely]] {
            panic("Failed to insert cap.");
          }

          region.start.value += 1ull << size_bit;
        } else {
          break;
        }
      }

      ++size_bit;
    }

    while (region.start < region.end) {
      size_t size = region.end.as<uintptr_t>() - region.start.as<uintptr_t>();

      if (size >= (1ull << size_bit)) {
        cap_t cap = make_memory_cap(flags, size_bit, region.start);
        logd(tag, "Memory capability created. addr=%p, size=%p(2^%-2d), type=%s", region.start, 1ull << size_bit, size_bit, flags & MEMORY_CAP_DEVICE ? "device" : "memory");

        if (!insert_cap(root_task, cap)) [[unlikely]] {
          panic("Failed to insert cap.");
        }

        region.start.value += 1ull << size_bit;
      } else {
        --size_bit;
      }
    }
  }
} // namespace

__init_code void setup_memory_capabilities(task_t* root_task, boot_info_t* boot_info) {
  assert(root_task != nullptr);
  assert(boot_info != nullptr);

  if constexpr (CONFIG_LOG_DEBUG) {
    dump_devices(boot_info->dtb);
  }

  reserved_region_count = 0;
  memory_region_count   = 0;
  device_region_count   = 0;

  push_reserved_region(region_t {
      .start = map_addr_t::from(_kernel_start).as_phys(),
      .end   = map_addr_t::from(_kernel_end).as_phys(),
  });

  push_reserved_region(region_t {
      .start = map_addr_t::from(_payload_start).as_phys(),
      .end   = map_addr_t::from(_payload_end).as_phys(),
  });

  for_each_dtb_node(boot_info->dtb, [](const dtb_node_t* node) {
    if (strcmp("cpus", node->name) == 0) {
      return false;
    }

    for_each_dtb_prop(node, [](const dtb_node_t* node, const dtb_prop_t* prop) {
      if (strcmp(prop->name, "reg") != 0) [[unlikely]] {
        return true;
      }

      // OpenSBI inserts its own reserved area into DTB as mmode_resv*.
      if (strncmp("mmode_resv", node->name, 10) == 0) {
        for_each_reg(node, prop, push_reserved_region);
      } else if (strcmp("memory", node->name) == 0) {
        for_each_reg(node, prop, push_memory_region);
      } else {
        for_each_reg(node, prop, push_device_region);
      }

      return false;
    });

    return true;
  });

  for (size_t i = 0; i < memory_region_count; ++i) {
    insert_memory_region_caps(root_task, memory_region[i], MEMORY_CAP_READABLE | MEMORY_CAP_WRITABLE | MEMORY_CAP_EXECUTABLE);
  }

  for (size_t i = 0; i < device_region_count; ++i) {
    insert_memory_region_caps(root_task, device_region[i], MEMORY_CAP_DEVICE | MEMORY_CAP_READABLE | MEMORY_CAP_WRITABLE);
  }
}
