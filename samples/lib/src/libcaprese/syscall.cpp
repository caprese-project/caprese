#include <caprese/task/syscall.h>

#include <libcaprese/syscall.h>

sysret_t sys_null() {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_NULL));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_debug_putchar(char ch) {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_DEBUG_PUTCHAR));
  asm volatile("mv a0, %0" : : "r"(ch));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_create_class() {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_CREATE_CLASS));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_create() {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_CREATE));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_call_method(uint64_t cid, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_CALL_METHOD));
  asm volatile("mv a0, %0" : : "r"(cid));
  asm volatile("mv a1, %0" : : "r"(method));
  asm volatile("mv a2, %0" : : "r"(arg0));
  asm volatile("mv a3, %0" : : "r"(arg1));
  asm volatile("mv a4, %0" : : "r"(arg2));
  asm volatile("mv a5, %0" : : "r"(arg3));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_get_field(uint64_t cid, uint8_t field) {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_GET_FIELD));
  asm volatile("mv a0, %0" : : "r"(cid));
  asm volatile("mv a1, %0" : : "r"(field));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_is_permitted(uint64_t cid, uint8_t permission) {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_IS_PERMITTED));
  asm volatile("mv a0, %0" : : "r"(cid));
  asm volatile("mv a1, %0" : : "r"(permission));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}

sysret_t sys_cap_list_size() {
  sysret_t sysret;
  asm volatile("li a7, %0" : : "i"(SYS_CAP_LIST_SIZE));
  asm volatile("ecall");
  asm volatile("mv %0, a0" : "=r"(sysret.result));
  asm volatile("mv %0, a1" : "=r"(sysret.error));
  return sysret;
}
