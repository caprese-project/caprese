#include <cstdio>
#include <cstdlib>

#include <runtime/cap.h>
#include <runtime/elf.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>
#include <runtime/task.h>
#include <runtime/types.h>

extern "C" {
  extern const char _apm_start[];
  extern const char _apm_end[];
}

int main(memory_handle_t boot_info) {
  printf("Hello, init task! boot_info: %d\n", boot_info);

  task_handle_t apm_handle = create_task();
  if (apm_handle == 0) [[unlikely]] {
    printf("Failed to create apm task.\n");
    abort();
  }

  if (!load_elf(apm_handle, _apm_start)) [[unlikely]] {
    printf("Failed to load apm task.\n");
    abort();
  }

  sysret_t sysret = sys_cap_list_size();
  if (sysret.error) [[unlikely]] {
    printf("Failed to load cap list size.\n");
    abort();
  }

  for (handle_t handle = 0; handle < sysret.result; ++handle) {
    if (get_handle_type(handle) != HANDLE_TYPE_MEMORY) {
      continue;
    }

    if (!is_mapped(handle)) {
      if (sys_cap_move(handle, apm_handle).error) [[unlikely]] {
        printf("Failed to move memory cap.\n");
        abort();
      }
    }
  }

  switch_task(apm_handle);

  while (true) {
    yield();
  }

  return 0;
}
