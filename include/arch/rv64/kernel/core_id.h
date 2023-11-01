#ifndef ARCH_RV64_KERNEL_CORE_ID_H_
#define ARCH_RV64_KERNEL_CORE_ID_H_

#include <cstdint>

using core_id_t = uintptr_t;

void      set_core_id(core_id_t core_id);
core_id_t get_core_id();

#endif // ARCH_RV64_KERNEL_CORE_ID_H_
