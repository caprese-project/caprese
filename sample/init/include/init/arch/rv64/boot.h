#ifndef INIT_ARCH_RV64_BOOT_H_
#define INIT_ARCH_RV64_BOOT_H_

#include <cstddef>
#include <cstdint>
#include <utility>

#include <lib/capability.h>

namespace init::inline rv64 {
  // mm, plic, fs
  constexpr size_t NUM_INIT_TASKS = 3;

  struct boot_t {
    uintptr_t    hartid;
    const char*  dtb;
    cap_handle_t init_task_cap;

    uintptr_t free_address_start;

    struct {
      const char* start;
      const char* end;
      void (*run)(boot_t* boot, cap_handle_t task_cap);
      cap_handle_t task_cap;
    } tasks[NUM_INIT_TASKS];
  };
} // namespace init::inline rv64

#endif // INIT_ARCH_RV64_BOOT_H_
