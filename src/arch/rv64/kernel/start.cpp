#include <cstdint>

#include <kernel/address.h>
#include <kernel/boot.h>
#include <kernel/start.h>

extern "C" {
  [[noreturn]] void arch_start(uintptr_t hartid, map_ptr<char> dtb) {
    boot_info_t boot_info;
    boot_info.core_id   = hartid;
    boot_info.cap_count = 0;
    boot_info.dtb       = dtb;

    start(make_map_ptr(&boot_info));
  }
}
