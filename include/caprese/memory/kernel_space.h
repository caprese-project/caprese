#ifndef CAPRESE_MEMORY_KERNEL_SPACE_H_
#define CAPRESE_MEMORY_KERNEL_SPACE_H_

#include <caprese/memory/address.h>

namespace caprese::memory {
  void init_kernel_space();

  void copy_kernel_space_mapping(mapped_address_t dst_page_table, mapped_address_t src_page_table);
}

#endif // CAPRESE_MEMORY_KERNEL_SPACE_H_
