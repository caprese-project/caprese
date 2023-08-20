#include <lib/debug.h>
#include <lib/syscall.h>

int main() {
  printd("Hello, mm! tid=%d\n", sys_task_tid().result);
  while (true) { }
  return 0;
}
