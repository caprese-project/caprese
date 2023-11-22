#include <kernel/cls.h>
#include <kernel/syscall.h>
#include <libcaprese/syscall.h>

void get_syscall_args(map_ptr<syscall_args_t> args) {
  assert(args != nullptr);

  map_ptr<task_t>& task = get_cls()->current_task;

  args->args[0] = task->frame.a0;
  args->args[1] = task->frame.a1;
  args->args[2] = task->frame.a2;
  args->args[3] = task->frame.a3;
  args->args[4] = task->frame.a4;
  args->args[5] = task->frame.a5;
  args->args[6] = task->frame.a6;
  args->code    = task->frame.a7;
}

sysret_t invoke_syscall_arch(uint16_t id, [[maybe_unused]] map_ptr<syscall_args_t> args) {
  switch (id) {
    case SYS_ARCH_MMU_MODE & 0xffff:
#if defined(CONFIG_MMU_SV39)
      return sysret_s_ok(RISCV_MMU_SV39);
#elif defined(CONFIG_MMU_SV48)
      return sysret_s_ok(RISCV_MMU_SV48);
#else
      return sysret_s_ok(0);
#endif
    default:
      return sysret_e_invalid_code();
  }
}
