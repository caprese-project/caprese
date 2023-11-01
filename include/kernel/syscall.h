#ifndef KERNEL_SYSCALL_H_
#define KERNEL_SYSCALL_H_

#include <cstdint>

struct sysret_t {
  intptr_t result;
  intptr_t error;
};

sysret_t invoke_syscall();

#endif // KERNEL_SYSCALL_H_
