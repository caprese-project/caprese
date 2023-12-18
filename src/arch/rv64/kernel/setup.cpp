#include <bit>
#include <cstring>
#include <iterator>
#include <utility>

#include <kernel/align.h>
#include <kernel/arch/dtb.h>
#include <kernel/cap.h>
#include <kernel/log.h>
#include <kernel/setup.h>

extern "C" {
  extern const char _kernel_start[];
  extern const char _kernel_end[];
  extern const char _payload_start[];
  extern const char _payload_end[];
  extern const char _root_task_stack_start[];
  extern const char _root_task_stack_end[];
}

namespace {
  constexpr const char* tag = "kernel/setup";

  struct region_t {
    phys_ptr<const char> start;
    phys_ptr<const char> end;
  };

  // a - b
  inline void subtract_region(region_t a, region_t b, region_t (&dst)[2]) {
    dst[0] = { .start = 0_phys, .end = 0_phys };
    dst[1] = { .start = 0_phys, .end = 0_phys };

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

  __init_code void dump_devices(map_ptr<char> dtb) {
    for_each_dtb_node(dtb, [](map_ptr<dtb_node_t> node) {
      logd(tag, "Device found: %s (%p)", node->name, node->unit_address);

      for_each_dtb_prop(node, []([[maybe_unused]] map_ptr<dtb_node_t>, map_ptr<dtb_prop_t> prop) {
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

  __init_code void for_each_reg(map_ptr<dtb_node_t> node, map_ptr<dtb_prop_t> prop, void (*callback)(region_t)) {
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
        .start = make_phys_ptr(base),
        .end   = make_phys_ptr(base + size),
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

  __init_code void insert_memory_region_caps(map_ptr<task_t> root_task, map_ptr<root_boot_info_t> root_boot_info, region_t region, bool device, size_t index = 0) {
    if (region.start >= region.end) [[unlikely]] {
      return;
    }

    if (index < reserved_region_count) {
      region_t dst[2];
      subtract_region(region, reserved_region[index], dst);
      for (region_t reg : dst) {
        if (reg.start < reg.end) {
          insert_memory_region_caps(root_task, root_boot_info, reg, device, index + 1);
        }
      }

      return;
    }

    uint16_t size_bit = 0;

    while (region.start < region.end) {
      size_t size = region.end.raw() - region.start.raw();

      if (region.start.raw() & (1ull << size_bit)) {
        if (size >= (1ull << size_bit)) {
          capability_t cap = make_memory_cap(device, 1ull << size_bit, region.start.as<void>());
          logd(tag, "Memory capability created. addr=%p, size=%p(2^%-2d), type=%s", region.start, 1ull << size_bit, size_bit, device ? "device" : "memory");

          map_ptr<cap_slot_t> cap_slot = insert_cap(root_task, cap);
          if (cap_slot == nullptr) [[unlikely]] {
            panic("Failed to insert cap.");
          }
          root_boot_info->caps[root_boot_info->mem_caps_offset + root_boot_info->num_mem_caps++] = get_cap_slot_index(cap_slot);

          region.start += 1ull << size_bit;
        } else {
          break;
        }
      }

      ++size_bit;
    }

    while (region.start < region.end) {
      size_t size = region.end.raw() - region.start.raw();

      if (size >= (1ull << size_bit)) {
        capability_t cap = make_memory_cap(device, 1ull << size_bit, region.start.as<void>());
        logd(tag, "Memory capability created. addr=%p, size=%p(2^%-2d), type=%s", region.start, 1ull << size_bit, size_bit, device ? "device" : "memory");

        map_ptr<cap_slot_t> cap_slot = insert_cap(root_task, cap);
        if (cap_slot == nullptr) [[unlikely]] {
          panic("Failed to insert cap.");
        }
        root_boot_info->caps[root_boot_info->mem_caps_offset + root_boot_info->num_mem_caps++] = get_cap_slot_index(cap_slot);

        region.start += 1ull << size_bit;
      } else {
        --size_bit;
      }
    }
  }
} // namespace

__init_code void setup_memory_capabilities(map_ptr<boot_info_t> boot_info) {
  assert(boot_info != nullptr);

  if constexpr (CONFIG_LOG_DEBUG) {
    dump_devices(boot_info->dtb);
  }

  reserved_region_count = 0;
  memory_region_count   = 0;
  device_region_count   = 0;

  push_reserved_region(region_t {
      .start = make_map_ptr(_kernel_start),
      .end   = make_map_ptr(_kernel_end),
  });

  push_reserved_region(region_t {
      .start = make_map_ptr(_payload_start),
      .end   = make_map_ptr(_payload_end),
  });

  push_reserved_region(region_t {
      .start = make_map_ptr(_root_task_stack_start),
      .end   = make_map_ptr(_root_task_stack_end),
  });

  auto [dtb_start, dtb_end] = get_dtb_region(boot_info->dtb);
  size_t dtb_size           = round_up(dtb_end - dtb_start, PAGE_SIZE);

  const region_t dtb_region {
    .start = dtb_start.as<const char>(),
    .end   = dtb_start.as<const char>() + dtb_size,
  };

  for_each_dtb_node(boot_info->dtb, [](map_ptr<dtb_node_t> node) {
    if (strcmp("cpus", node->name) == 0) {
      return false;
    }

    for_each_dtb_prop(node, [](map_ptr<dtb_node_t> node, map_ptr<dtb_prop_t> prop) {
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

  map_ptr<task_t> root_task = boot_info->root_task;

  for (size_t i = 0; i < memory_region_count; ++i) {
    region_t dst[2];
    subtract_region(memory_region[i], dtb_region, dst);
    for (auto& region : dst) {
      if (region.start < region.end) {
        insert_memory_region_caps(root_task, boot_info->root_boot_info, region, false);
      }
    }
  }

  for (size_t i = 0; i < device_region_count; ++i) {
    insert_memory_region_caps(root_task, boot_info->root_boot_info, device_region[i], true);
  }
}

__init_code void setup_arch_root_boot_info(map_ptr<boot_info_t> boot_info) {
  assert(boot_info != nullptr);

  map_ptr<root_boot_info_t> root_boot_info = boot_info->root_boot_info;

  auto [start, end] = get_dtb_region(boot_info->dtb);

  root_boot_info->arch_info.dtb_start          = start.raw();
  root_boot_info->arch_info.dtb_end            = end.raw();
  root_boot_info->arch_info.num_dtb_vp_caps    = 0;
  root_boot_info->arch_info.dtb_vp_caps_offset = root_boot_info->mem_caps_offset + root_boot_info->num_mem_caps;

  map_ptr<page_table_t> page_table   = boot_info->payload_page_tables[KILO_PAGE_TABLE_LEVEL];
  size_t                dtb_size     = static_cast<size_t>(end - start);
  size_t                payload_size = static_cast<size_t>(_payload_end - _payload_start);
  virt_ptr<void>        va_base      = make_virt_ptr(CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS + payload_size);

  for (uintptr_t va_offset = 0; va_offset < dtb_size; va_offset += PAGE_SIZE) {
    map_ptr<void> page = (start + va_offset).as_map();

    map_ptr<pte_t> pte = page_table->walk(va_base + va_offset, KILO_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 0, .executable = 0, .user = 1, .global = 0 });
    pte->set_next_page(page);
    pte->enable();

    map_ptr<cap_slot_t> virt_page_cap_slot = insert_cap(boot_info->root_task, make_virt_page_cap(true, false, false, KILO_PAGE_TABLE_LEVEL, page.as_phys().raw()));
    if (virt_page_cap_slot == nullptr) [[unlikely]] {
      panic("Failed to insert the virtual page capability.");
    }

    root_boot_info->caps[root_boot_info->arch_info.dtb_vp_caps_offset + root_boot_info->arch_info.num_dtb_vp_caps++] = get_cap_slot_index(virt_page_cap_slot);

    logd(tag, "Mapped page %p -> %p (4k, dtb)", va_base.raw() + va_offset, page.as_phys());
  }
}

__init_code void* bake_stack(map_ptr<void> stack, map_ptr<void> data, size_t size) {
  assert(stack != nullptr);
  assert(data != nullptr);
  assert(size > 0);

  uintptr_t top = round_down(stack.raw() - size, 16);

  memcpy(reinterpret_cast<void*>(top), data.get(), size);

  return reinterpret_cast<void*>(top);
}
