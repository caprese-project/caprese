#ifndef LIB_SYSCALL_H_
#define LIB_SYSCALL_H_

#include <cstdint>

extern "C" {
  struct sysret_t {
    uintptr_t result;
    uintptr_t error;
  };

  sysret_t sys_base_null();
  sysret_t sys_base_core_id();

  sysret_t sys_debug_putchar(char ch);

  sysret_t sys_cap_create_class();
  sysret_t sys_cap_create();
  sysret_t sys_cap_call_method(uint64_t cid, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
  sysret_t sys_cap_get_field(uint64_t cid, uint8_t field);
  sysret_t sys_cap_is_permitted(uint64_t cid, uint8_t permission);
  sysret_t sys_cap_list_base();
  sysret_t sys_cap_list_size();
}

#endif // LIB_SYSCALL_H_
