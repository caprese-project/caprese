#include <cstdint>

#include <kernel/cls.h>
#include <kernel/csr.h>
#include <kernel/syscall.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <log/log.h>

namespace {
  constexpr const char* tag = "kernel/trap";
} // namespace

extern "C" {
  // Defined in src/arch/rv64/kernel/trap.S
  [[noreturn]] void _return_to_user_mode();

  [[noreturn]] void _kernel_trap() {
    panic("Kernel trap!");
  }

  [[noreturn]] void _user_trap() {
    uint64_t scause;
    asm volatile("csrr %0, scause" : "=r"(scause));

    task_t* task = get_cls()->current_task;

    if (scause & SCAUSE_INTERRUPT) {
      logd(tag, "scause-interrupt: %p", scause & SCAUSE_EXCEPTION_CODE);
      if ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT) {
      } else {
        panic("User trap!");
      }
    } else {
      if (scause & SCAUSE_ENVIRONMENT_CALL_FROM_U_MODE) {
        enable_trap();
        sysret_t sysret               = invoke_syscall();
        task->arch_task.trap_frame.a0 = sysret.result;
        task->arch_task.trap_frame.a1 = sysret.error;
        task->arch_task.trap_frame.sepc += 4;
      } else {
        panic("User trap!");
      }
    }

    return_to_user_mode();
  }
}

[[noreturn]] void return_to_user_mode() {
  logd(tag, "Return to user mode.");

  uint64_t sstatus;
  asm volatile("csrr %0, sstatus" : "=r"(sstatus));
  sstatus &= ~SSTATUS_SIE;
  sstatus &= ~SSTATUS_SPP;
  sstatus |= SSTATUS_SPIE;
  asm volatile("csrw sstatus, %0" : : "r"(sstatus));

  _return_to_user_mode();
}

[[noreturn]] void default_trap_handler() {
  logd(tag, "Handle trap!");
  return_to_user_mode();
}

void set_trap_handler(void (*handler)()) {
  asm volatile("csrw stvec, %0" : : "r"(handler));
}

void enable_trap() {
  uint64_t sie;
  asm volatile("csrr %0, sie" : "=r"(sie));
  sie |= SIE_SEIE;
  sie |= SIE_STIE;
  sie |= SIE_SSIE;
  asm volatile("csrw sie, %0" : : "r"(sie));
}

void disable_trap() {
  uint64_t sie;
  asm volatile("csrr %0, sie" : "=r"(sie));
  sie &= ~SIE_SEIE;
  sie &= ~SIE_STIE;
  sie &= ~SIE_SSIE;
  asm volatile("csrw sie, %0" : : "r"(sie));
}
