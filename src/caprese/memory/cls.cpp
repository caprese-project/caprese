#include <algorithm>
#include <cstdio>
#include <cstring>

#include <caprese/arch/device.h>
#include <caprese/arch/system.h>
#include <caprese/memory/cls.h>

extern "C" {
  caprese::memory::core_local_storage_t _core_local_storage_table[CONFIG_MAX_CORES];
}

namespace caprese::memory {
  void init_cls_space(const arch::boot_info_t* boot_info) {
    size_t max_core_id = 0;
    arch::scan_device(boot_info, [&max_core_id](arch::scan_callback_args_t* args) {
      if (strncmp(args->device_name, "cpu", 3) == 0) [[unlikely]] {
        max_core_id = std::max<size_t>(max_core_id, args->address);
      }
    });

    printf("Max core id: %lu\n", max_core_id);

    for (size_t core_id = 0; core_id < max_core_id; ++core_id) {
      _core_local_storage_table[core_id]         = {};
      _core_local_storage_table[core_id].core_id = core_id;
    }
  }

  core_local_storage_t* get_cls() {
    auto core_id = arch::get_core_id();
    if (core_id >= CONFIG_MAX_CORES) [[unlikely]] {
      return nullptr;
    }
    return _core_local_storage_table + core_id;
  }
} // namespace caprese::memory
