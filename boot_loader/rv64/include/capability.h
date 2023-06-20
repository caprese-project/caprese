/**
 * @file capability.h
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

#ifndef BOOT_LOADER_CAPABILITY_H_
#define BOOT_LOADER_CAPABILITY_H_

#include <caprese/capability/capability.h>

#include <libcaprese/device/device_tree.h>

namespace caprese::boot_loader {
  struct capability_table_wrapper {
    capability_table_wrapper* prev;
    capability::capability_table_t*       table;
  };

  const capability_table_wrapper* get_current_capability_table();

  void insert_capability(capability::capability_t cap);

  void create_memory_capabilities(const device_tree_node_t& node);
  void create_irq_capabilities(const device_tree_node_t& node);
} // namespace caprese::boot_loader

#endif // BOOT_LOADER_CAPABILITY_H_
