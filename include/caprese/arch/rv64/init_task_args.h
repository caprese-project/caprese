#ifndef CAPRESE_ARCH_RV64_INIT_TASK_ARGS_H_
#define CAPRESE_ARCH_RV64_INIT_TASK_ARGS_H_

#include <cstddef>

#include <caprese/task/task.h>

namespace caprese::arch::inline rv64 {
  struct init_task_args_t {
    task::cid_handle_t init_task_cid_handle;
    uintptr_t          free_address_start;
    size_t             num_device_tree_blob_cid_handles;
    task::cid_handle_t device_tree_blob_cid_handles[];
  };
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_INIT_TASK_ARGS_H_
