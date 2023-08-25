#ifndef RUNTIME_GLOBAL_H_
#define RUNTIME_GLOBAL_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  extern task_handle_t __init_task_handle;
  extern task_handle_t __this_task_handle;
  extern task_handle_t __apm_task_handle;
  extern task_handle_t __mm_task_handle;
  extern task_handle_t __fs_task_handle;
  extern task_handle_t __console_task_handle;

  extern uintptr_t __heap_start;

  extern size_t    __page_size;
  extern uintptr_t __user_space_start;
  extern uintptr_t __user_space_end;

  extern uint64_t* __handle_list_base;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_GLOBAL_H_
