/**
 * @file boot_info.h
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

#ifndef BOOT_LOADER_BOOT_INFO_H_
#define BOOT_LOADER_BOOT_INFO_H_

#include <cstdint>

namespace caprese::boot_loader {
  struct region {
    uint64_t begin;
    uint64_t end;
  };

  struct boot_info_t {
    uint64_t  hartid;
    uintptr_t device_tree_blob;
    uintptr_t root_page_table;
    size_t    nhart;
    size_t    total_memory_size;
    region    used_regions[];
  };
} // namespace caprese::boot_loader

#endif // BOOT_LOADER_BOOT_INFO_H_
