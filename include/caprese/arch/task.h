#ifndef CAPRESE_ARCH_TASK_H_
#define CAPRESE_ARCH_TASK_H_

#if defined(CONFIG_ARCH_RISCV) && defined(CONFIG_XLEN_64)
#include <caprese/arch/rv64/task.h>
#endif

#endif // CAPRESE_ARCH_TASK_H_
