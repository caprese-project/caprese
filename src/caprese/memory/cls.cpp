#include <algorithm>
#include <cstdio>
#include <cstring>

#include <caprese/arch/device.h>
#include <caprese/arch/system.h>
#include <caprese/memory/cls.h>

namespace caprese::memory {
  namespace {
    core_local_storage_t core_local_storage_table[CONFIG_MAX_CORES];
  }; // namespace

  void init_cls_space(const arch::boot_info_t* boot_info) {
    int max_core_id = 0;
    arch::scan_device(boot_info, [&max_core_id](arch::scan_callback_args_t* args) {
      if (strncmp(args->device_name, "cpu", 3) == 0) [[unlikely]] {
        max_core_id = std::max<int>(max_core_id, args->address);
      }
    });

    printf("Max core id: %d\n", max_core_id);

    for (int core_id = 0; core_id < max_core_id; ++core_id) {
      core_local_storage_table[core_id] = {};
    }
  }

  core_local_storage_t* get_cls() {
    auto core_id = arch::get_core_id();
    if (core_id >= CONFIG_MAX_CORES) [[unlikely]] {
      return nullptr;
    }
    return core_local_storage_table + core_id;
  }
} // namespace caprese::memory
