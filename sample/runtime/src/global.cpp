#include <runtime/global.h>

extern "C" {
  task_handle_t __init_task_handle;
  task_handle_t __this_task_handle;
  task_handle_t __apm_task_handle;
  task_handle_t __mm_task_handle;
  task_handle_t __fs_task_handle;
  task_handle_t __console_task_handle;

  uintptr_t __heap_start;

  size_t    __page_size;
  uintptr_t __user_space_start;
  uintptr_t __user_space_end;

  uint64_t* __handle_list_base;

  void* __dso_handle;
}
