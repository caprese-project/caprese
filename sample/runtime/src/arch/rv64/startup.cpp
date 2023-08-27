#include <cstdlib>

#include <runtime/global.h>
#include <runtime/syscall.h>
#include <runtime/task.h>
#include <runtime/types.h>

extern "C" {
  void __init_exit_procs();
  void __init_heap();
  bool __init_console();

  extern void (*__init_array_start[])();
  extern void (*__init_array_end[])();
  extern void (*__fini_array_start[])();
  extern void (*__fini_array_end[])();
}

namespace {
  void cleanup() {
    for (void (**destructor)() = __fini_array_start; destructor != __fini_array_end; ++destructor) {
      (*destructor)();
    }
  }
} // namespace

extern "C" int __runtime_startup_routine(task_handle_t init_task_handle, task_handle_t this_task_handle, task_handle_t apm_task_handle, uintptr_t heap_start) {
  __init_task_handle    = init_task_handle;
  __this_task_handle    = this_task_handle;
  __apm_task_handle     = apm_task_handle;
  __mm_task_handle      = 0;
  __fs_task_handle      = 0;
  __console_task_handle = 0;

  __heap_start = heap_start;

  __init_exit_procs();
  if (atexit(cleanup) != 0) [[unlikely]] {
    return 1;
  }

  sysret_t sysret;

  sysret = sys_base_page_size();
  if (sysret.error) [[unlikely]] {
    return 1;
  }
  __page_size = sysret.result;

  sysret = sys_base_user_space_start();
  if (sysret.error) [[unlikely]] {
    return 1;
  }
  __user_space_start = sysret.result;

  sysret = sys_base_user_space_end();
  if (sysret.error) [[unlikely]] {
    return 1;
  }
  __user_space_end = sysret.result;

  sysret = sys_cap_list_base();
  if (sysret.error) [[unlikely]] {
    return 1;
  }
  __handle_list_base = reinterpret_cast<uint64_t*>(sysret.result);

  __init_heap();

  if (!__init_console()) [[unlikely]] {
    return 1;
  }

  if (apm_task_handle != 0) {
    // __console_task_handle = lookup_task("console");
    // __mm_task_handle      = lookup_task("mm");
    // __fs_task_handle      = lookup_task("fs");
  }

  for (void (**constructor)() = __init_array_start; constructor != __init_array_end; ++constructor) {
    (*constructor)();
  }

  return 0;
}
