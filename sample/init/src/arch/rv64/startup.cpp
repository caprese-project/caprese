#include <caprese/arch/init_task_args.h>

#include <runtime/memory.h>
#include <runtime/types.h>

struct init_startup_routine_result_t {
  uintptr_t init_task_handle;
  uintptr_t heap_start;
};

extern "C" {
  init_startup_routine_result_t __init_startup_routine(memory_handle_t init_args_handle) {
    caprese::arch::init_task_args_t* init_args = reinterpret_cast<caprese::arch::init_task_args_t*>(get_virtual_address(init_args_handle));

    return {
      .init_task_handle = init_args->init_task_cid_handle,
      .heap_start       = init_args->free_address_start,
    };
  }
}
