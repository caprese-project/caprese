#ifndef CAPRESE_TASK_SYSCALL_H_
#define CAPRESE_TASK_SYSCALL_H_

#define SYS_NULL             0
#define SYS_DEBUG_PUTCHAR    1
#define SYS_CAP_CREATE_CLASS 2
#define SYS_CAP_CREATE       3
#define SYS_CAP_CALL_METHOD  4
#define SYS_CAP_GET_FIELD    5
#define SYS_CAP_IS_PERMITTED 6
#define SYS_CAP_LIST_SIZE    7

#ifndef ASM

#include <cstdint>

namespace caprese::task {
  struct sysret_t {
    uintptr_t result;
    uintptr_t error;
  };

  namespace syscall {
    sysret_t null(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t debug_putchar(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_create_class(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_create(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_call_method(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_get_field(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_is_permitted(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
    sysret_t cap_list_size(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
  } // namespace syscall

  [[nodiscard]] sysret_t handle_system_call(uintptr_t code, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);
} // namespace caprese::task

#endif // ASM

#endif // CAPRESE_TASK_SYSCALL_H_
