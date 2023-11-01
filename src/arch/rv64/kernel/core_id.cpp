#include <kernel/core_id.h>

void set_core_id(core_id_t core_id) {
  asm volatile("mv tp, %0" : : "r"(core_id));
}

core_id_t get_core_id() {
  core_id_t core_id;
  asm volatile("mv %0, tp" : "=r"(core_id));
  return core_id;
}
