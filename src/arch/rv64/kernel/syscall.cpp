#include <kernel/syscall.h>

void get_syscall_args(task_t* task, syscall_args_t* args) {
  assert(args != nullptr);

  args->args[0] = task->frame.a0;
  args->args[1] = task->frame.a1;
  args->args[2] = task->frame.a2;
  args->args[3] = task->frame.a3;
  args->args[4] = task->frame.a4;
  args->args[5] = task->frame.a5;
  args->args[6] = task->frame.a6;
  args->code    = task->frame.a7;
}

sysret_t invoke_syscall_arch(task_t* task, uint16_t id, syscall_args_t* args) {
  (void)task;
  (void)id;
  (void)args;
  return sysret_e_invalid_code();
}
