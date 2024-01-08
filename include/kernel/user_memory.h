#ifndef KERNEL_USER_MEMORY_H_
#define KERNEL_USER_MEMORY_H_

#include <kernel/task.h>

bool read_user_memory(map_ptr<task_t> task, uintptr_t src, map_ptr<void> dst, size_t size);
bool write_user_memory(map_ptr<task_t> task, map_ptr<void> src, uintptr_t dst, size_t size);
bool forward_user_memory(map_ptr<task_t> src_task, map_ptr<task_t> dst_task, uintptr_t src, uintptr_t dst, size_t size);

#endif // KERNEL_USER_MEMORY_H_
