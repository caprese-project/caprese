#ifndef ARCH_RV64_KERNEL_SETUP_H_
#define ARCH_RV64_KERNEL_SETUP_H_

#include <kernel/attribute.h>
#include <kernel/boot.h>
#include <kernel/cap_space.h>
#include <kernel/task.h>

__init_code void setup_memory_capabilities(task_t* root_task, boot_info_t* boot_info);

#endif // ARCH_RV64_KERNEL_SETUP_H_
