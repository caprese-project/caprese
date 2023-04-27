/**
 * @file main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement platform-independent kernel main.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <caprese/kernel/capability.h>
#include <caprese/kernel/device.h>
#include <caprese/kernel/main.h>
#include <caprese/kernel/panic.h>
#include <caprese/lib/console.h>
#include <caprese/lib/endian.h>

namespace caprese {
  namespace {
    device          dev;
    capability_pool cap_pool;
  } // namespace

  [[noreturn]] void main(uintptr_t hartid, const void* dtb) {
    log_info("kernel", "Kernel is booting on hart id %d...", hartid);
    log_debug("kernel", "DTB installation address: %p", dtb);

    if (failed(device::from_dtb(dev, reinterpret_cast<const char*>(dtb)))) [[unlikely]] {
      log_fatal("kernel", "Failed to load device tree.");
    }

    if (failed(capability_pool::create(cap_pool, dev))) [[unlikely]] {
      log_fatal("kernel", "Failed to create capability pool.");
    }

    // device_node root_device_node;
    // if (failed(dev.get_root_node(root_device_node))) [[unlikely]] {
    //   log_fatal("kernel", "Failed to get root device node.");
    // }

    // device_node memory_device_node;
    // if (failed(root_device_node.find_node_by_name(memory_device_node, "memory"))) [[unlikely]] {
    //   log_fatal("kernel", "Failed to get memory device node.");
    // }

    // device_node_property memory_region;
    // memory_device_node.find_property_by_name(memory_region, "reg");
    // for (size_t i = 0; i < memory_region.data_length; i += sizeof(uintptr_t) * 2) {
    //   uintptr_t memory_region_begin = swap_endian(*reinterpret_cast<const uintptr_t*>(memory_region.data + i));
    //   uintptr_t memory_region_size = swap_endian(*reinterpret_cast<const uintptr_t*>(memory_region.data + i + sizeof(uintptr_t)));
    //   uintptr_t memory_region_end = memory_region_begin + memory_region_size;
    //   log_info("kernel", "Available memory space has been detected. begin: %p, end: %p, size: %p", memory_region_begin, memory_region_end,
    //   memory_region_size);

    //   capability::register_memory_region(dev, memory_region_begin, memory_region_size);
    // }

    // create caps

    // init root server

    // run root server

    // unreachable

    panic("TEST PANIC");
  }
} // namespace caprese
