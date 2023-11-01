#include <cstdint>

#include <kernel/address.h>
#include <kernel/boot.h>
#include <kernel/start.h>
#include <log/log.h>

namespace {
  constexpr const char* tag = "kernel/start";
} // namespace

extern "C" {
  [[noreturn]] void arch_start(uintptr_t hartid, map_addr_t dtb) {
    lognl();
    logd(tag, "Boot args: hartid=%lu, dtb=%p", hartid, dtb);

    boot_info_t boot_info;
    boot_info.hartid = hartid;
    boot_info.dtb    = dtb;

    start(&boot_info);
  }
}
