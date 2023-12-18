#include <cstdint>

#include <kernel/address.h>
#include <kernel/boot_info.h>
#include <kernel/start.h>

extern "C" {
  [[noreturn]] void arch_start(uintptr_t hartid, map_ptr<char> dtb) {
    init_boot_info(hartid, dtb);
    start();
  }
}
