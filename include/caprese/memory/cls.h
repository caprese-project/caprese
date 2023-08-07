#ifndef CAPRESE_MEMORY_CLS_H_
#define CAPRESE_MEMORY_CLS_H_

#include <caprese/arch/boot_info.h>

namespace caprese::memory {
  struct core_local_storage_t {};

  void init_cls_space(const arch::boot_info_t* boot_info);
  core_local_storage_t* get_cls();
}

#endif // CAPRESE_MEMORY_CLS_H_
