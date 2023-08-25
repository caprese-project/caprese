#ifndef RUNTIME_SYS_BASE_H_
#define RUNTIME_SYS_BASE_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  sysret_t sys_base_null(void);
  sysret_t sys_base_core_id(void);
  sysret_t sys_base_page_size(void);
  sysret_t sys_base_user_space_start(void);
  sysret_t sys_base_user_space_end(void);

  sysret_t sys_debug_putchar(char ch);
  sysret_t sys_debug_getchar(void);

  sysret_t sys_cap_create_class(void);
  sysret_t sys_cap_create(void);
  sysret_t sys_cap_call_method(handle_t handle, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
  sysret_t sys_cap_get_field(handle_t handle, uint8_t field);
  sysret_t sys_cap_is_permitted(handle_t handle, uint8_t permission);
  sysret_t sys_cap_list_base(void);
  sysret_t sys_cap_list_size(void);
  sysret_t sys_cap_move(handle_t handle, task_handle_t dst_task_cap_handle);
  sysret_t sys_cap_copy(handle_t handle, uint64_t permissions);

  sysret_t sys_task_tid(void);
  sysret_t sys_task_create(void);
  sysret_t sys_task_yield(void);
  sysret_t sys_task_ipc_receive(ipc_msg_t* msg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_SYS_BASE_H_
