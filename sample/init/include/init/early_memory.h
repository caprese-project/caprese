#ifndef INIT_EARLY_MEMORY_H_
#define INIT_EARLY_MEMORY_H_

#include <cstddef>

#include <init/arch/boot.h>
#include <lib/capability.h>

namespace init {
  bool         init_early_memory(boot_t* boot);
  cap_handle_t fetch_mem_cap();
  void*        early_alloc(size_t size);
  void         early_free_all();
} // namespace init

#endif // INIT_EARLY_MEMORY_H_
