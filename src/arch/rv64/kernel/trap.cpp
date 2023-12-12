#include <cstdint>

#include <kernel/arch/csr.h>
#include <kernel/cls.h>
#include <kernel/syscall.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <log/log.h>

namespace {
  constexpr const char* tag = "kernel/trap";
} // namespace

extern "C" {
  // Defined in src/arch/rv64/kernel/trap.S
  [[noreturn]] void _return_to_user_mode(frame_t*);

  [[noreturn]] void _kernel_trap() {
    panic("Kernel trap!");
  }

  [[noreturn]] void _user_trap() {
    uint64_t scause;
    asm volatile("csrr %0, scause" : "=r"(scause));

    map_ptr<task_t> cur_task = get_cls()->current_task;

    if (scause & SCAUSE_INTERRUPT) {
      logd(tag, "scause-interrupt: %p", scause & SCAUSE_EXCEPTION_CODE);
      if ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT) {
      } else {
        panic("User trap! tid=0x%x", cur_task->tid);
      }
    } else {
      if ((scause & SCAUSE_EXCEPTION_CODE) == SCAUSE_ENVIRONMENT_CALL_FROM_U_MODE) {
        enable_trap();

        sysret_t sysret = invoke_syscall();

        map_ptr<task_t>& task = get_cls()->current_task;
        task->frame.a0        = sysret.result;
        task->frame.a1        = sysret.error;
        task->frame.sepc += 4;
      } else {
        logd(tag, "scause-exception: %p", scause & SCAUSE_EXCEPTION_CODE);
        panic("User trap! tid=0x%x", cur_task->tid);
      }
    }

    return_to_user_mode();
  }
}

[[noreturn]] void return_to_user_mode() {
  map_ptr<task_t>& task = get_cls()->current_task;

  uint64_t sstatus;
  asm volatile("csrr %0, sstatus" : "=r"(sstatus));
  sstatus &= ~SSTATUS_SIE;
  sstatus &= ~SSTATUS_SPP;
  sstatus |= SSTATUS_SPIE;
  asm volatile("csrw sstatus, %0" : : "r"(sstatus));

  task->frame.stack = task.raw() + PAGE_SIZE;

  _return_to_user_mode(&task->frame);
}

void arch_init_task(map_ptr<task_t> task, void (*payload)()) {
  assert(task != nullptr);

  task->context = {};
  task->frame   = {};

  task->context.ra = reinterpret_cast<uintptr_t>(payload);
  task->context.sp = task.raw() + PAGE_SIZE;

#if defined(CONFIG_MMU_SV39)
  task->frame.satp = SATP_MODE_SV39;
#elif defined(CONFIG_MMU_SV48)
  task->frame.satp = SATP_MODE_SV48;
#endif

  task->frame.satp |= task->root_page_table.as_phys().raw() >> PAGE_SIZE_BIT;
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
