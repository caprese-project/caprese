#include <kernel/syscall.h>

sysret_t invoke_syscall() {
  return {
    .result = 0,
    .error  = 0,
  };
}
