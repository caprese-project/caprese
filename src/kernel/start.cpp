#include <kernel/attribute.h>
#include <kernel/start.h>
#include <log/log.h>

__init_code [[noreturn]] void start([[maybe_unused]] const boot_info_t* boot_info) {
  panic("Kernel not implemented");
}
