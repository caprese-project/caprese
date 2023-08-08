#ifndef CAPRESE_MEMORY_CLS_H_
#define CAPRESE_MEMORY_CLS_H_

#include <caprese/arch/boot_info.h>
#include <caprese/task/task.h>

namespace caprese::memory {
  struct alignas(CONFIG_CLS_SIZE) core_local_storage_t {
    size_t      core_id;
    task::tid_t current_tid;
  };

  static_assert(sizeof(core_local_storage_t) == (CONFIG_CLS_SIZE));

  void                  init_cls_space(const arch::boot_info_t* boot_info);
  core_local_storage_t* get_cls();
} // namespace caprese::memory

extern "C" {
  extern caprese::memory::core_local_storage_t _core_local_storage_table[CONFIG_MAX_CORES];
}

#endif // CAPRESE_MEMORY_CLS_H_
