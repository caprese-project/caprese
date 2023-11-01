#ifndef KERNEL_TASK_H_
#define KERNEL_TASK_H_

#include <kernel/arch_task.h>
#include <kernel/page.h>

struct task_t {
  arch_task_t   arch_task;
  page_table_t* root_page_table;
};

#endif // KERNEL_TASK_H_
