#define ASM
#include <caprese/task/syscall.h>

.section .text

.global sys_null
.type sys_null, @function
sys_null:
  li a7, SYS_NULL
  ecall
  ret

.global sys_debug_putchar
.type sys_debug_putchar, @function
sys_debug_putchar:
  li a7, SYS_DEBUG_PUTCHAR
  ecall
  ret

.global sys_cap_call_method
.type sys_cap_call_method, @function
sys_cap_call_method:
  li a7, SYS_CAP_CALL_METHOD
  ecall
  ret

.global sys_cap_get_field
.type sys_cap_get_field, @function
sys_cap_get_field:
  li a7, SYS_CAP_GET_FIELD
  ecall
  ret

.global sys_cap_is_permitted
.type sys_cap_is_permitted, @function
sys_cap_is_permitted:
  li a7, SYS_CAP_IS_PERMITTED
  ecall
  ret

.global sys_cap_list_size
.type sys_cap_list_size, @function
sys_cap_list_size:
  li a7, SYS_CAP_LIST_SIZE
  ecall
  ret
