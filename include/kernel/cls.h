#ifndef KERNEL_CLS_H_
#define KERNEL_CLS_H_

#include <kernel/task.h>

struct core_local_storage_t {
  map_ptr<task_t> current_task;
};

map_ptr<core_local_storage_t> get_cls();

#endif // KERNEL_CLS_H_
