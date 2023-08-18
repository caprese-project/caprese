#include <cstddef>
#include <cstdint>
#include <debug.h>
#include <syscall.h>
#include <capability.h>

extern "C" void start(uintptr_t hartid, const char* dtb) {
  printd("hartid: 0x%lx\n", hartid);
  printd("dtb: 0x%lx\n", dtb);

  uintptr_t list_base = sys_cap_list_base().result;
  size_t    list_size = sys_cap_list_size().result;
  printd("cap list: base=0x%lx, size=0x%lx\n", list_base, list_size);
}
