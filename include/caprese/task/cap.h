#ifndef CAPRESE_TASK_CAP_H_
#define CAPRESE_TASK_CAP_H_

#include <caprese/task/types.h>

namespace caprese::task {
  [[nodiscard]] cid_t                     make_cid(capability::capability_t* cap);
  [[nodiscard]] cid_t*                    lookup_cid(task_t* task, cid_handle_t handle);
  [[nodiscard]] capability::capability_t* lookup_capability(task_t* task, cid_t cid);
  cid_handle_t                            insert_capability(task_t* task, capability::capability_t* cap);
  cid_handle_t                            move_capability(task_t* dst_task, task_t* src_task, cid_handle_t handle);
  size_t                                  allocated_cap_list_size(task_t* task);
} // namespace caprese::task

#endif // CAPRESE_TASK_CAP_H_
