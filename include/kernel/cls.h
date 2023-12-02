#ifndef KERNEL_CLS_H_
#define KERNEL_CLS_H_

#include <kernel/task.h>

struct core_local_storage_t {
  alignas(PAGE_SIZE) char idle_task_root_page_table[PAGE_SIZE];
  alignas(PAGE_SIZE) char idle_task_region[PAGE_SIZE];
  map_ptr<task_t> idle_task;
  map_ptr<task_t> current_task;
  int             errno_value;
};

map_ptr<core_local_storage_t> get_cls();

#endif // KERNEL_CLS_H_
