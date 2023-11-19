#ifndef ARCH_RV64_KERNEL_TRAP_H_
#define ARCH_RV64_KERNEL_TRAP_H_

#include <kernel/task.h>

[[noreturn]] void return_to_user_mode();

void arch_init_task(map_ptr<task_t> task);

void set_trap_handler(void (*handler)());

void enable_trap();
void disable_trap();

#endif // ARCH_RV64_KERNEL_TRAP_H_
