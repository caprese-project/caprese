/**
 * @file trap.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Defines functions related to traps.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#include <caprese/lib/console.h>

#include <caprese/arch/riscv/thread_control_block.h>
#include <caprese/arch/riscv/trap.h>

extern "C" {
  void kernel_trap() {
    uintptr_t sepc, sstatus, scause;
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    asm volatile("csrr %0, scause" : "=r"(scause));

    caprese::log_debug("trap", "kernel_trap: %p", scause);
    // TODO: handle trap

    asm volatile("csrw sepc, %0" : : "r"(sepc));
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
  }
}

namespace caprese::arch {
  namespace {
    void user_trap() {
      asm volatile("csrw stvec, %0" : : "r"(kernel_vector));

      auto tcb = thread_control_block::current();
      asm volatile("csrr %0, sepc" : "=r"(tcb->trap_frame->epc));

      riscv_trap_code scause;
      asm volatile("csrr %0, scause" : "=r"(scause));

      // TODO: handle trap
      switch (scause) {
        using enum riscv_trap_code;
        case INSTRUCTION_ADDRESS_MISALIGNED:
          log_info("trap", "INSTRUCTION_ADDRESS_MISALIGNED, %p");
          break;
        case INSTRUCTION_ACCESS_FAULT:
          log_info("trap", "INSTRUCTION_ACCESS_FAULT");
          break;
        case ILLEGAL_INSTRUCTION:
          log_info("trap", "ILLEGAL_INSTRUCTION");
          break;
        case BREAKPOINT:
          log_info("trap", "BREAKPOINT");
          break;
        case LOAD_ADDRESS_MISALIGNED:
          log_info("trap", "LOAD_ADDRESS_MISALIGNED");
          break;
        case LOAD_ACCESS_FAULT:
          log_info("trap", "LOAD_ACCESS_FAULT");
          break;
        case STORE_AMO_ADDRESS_MISALIGNED:
          log_info("trap", "STORE_AMO_ADDRESS_MISALIGNED");
          break;
        case STORE_AMO_ACCESS_FAULT:
          log_info("trap", "STORE_AMO_ACCESS_FAULT");
          break;
        case ENVIRONMENT_CALL_FROM_U_MODE: {
          auto tcb = thread_control_block::current();
          print("%c", tcb->trap_frame->a0);
          tcb->trap_frame->epc += 4;
        } break;
        case ENVIRONMENT_CALL_FROM_S_MODE:
          log_info("trap", "ENVIRONMENT_CALL_FROM_S_MODE");
          break;
        case ENVIRONMENT_CALL_FROM_M_MODE:
          log_info("trap", "ENVIRONMENT_CALL_FROM_M_MODE");
          break;
        case INSTRUCTION_PAGE_FAULT:
          log_info("trap", "INSTRUCTION_PAGE_FAULT");
          break;
        case LOAD_PAGE_FAULT:
          log_info("trap", "LOAD_PAGE_FAULT");
          break;
        case STORE_AMO_PAGE_FAULT:
          log_info("trap", "STORE_AMO_PAGE_FAULT");
          break;
        default:
          log_error("trap", "Unknown scause: %p", scause);
      }

      return_to_user_mode();
    }
  } // namespace

  void return_to_user_mode() {
    auto tcb = thread_control_block::current();

    const auto user_vector = reinterpret_cast<trampoline_user_vector_t>(
        TRAMPOLINE_BASE_ADDRESS + (reinterpret_cast<uintptr_t>(&trampoline_user_vector) - reinterpret_cast<uintptr_t>(&begin_of_trampoline)));
    const auto return_to_user_mode = reinterpret_cast<trampoline_return_to_user_mode_t>(
        TRAMPOLINE_BASE_ADDRESS + (reinterpret_cast<uintptr_t>(&trampoline_return_to_user_mode) - reinterpret_cast<uintptr_t>(&begin_of_trampoline)));

    uintptr_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw stvec, %0" : : "r"(user_vector));
    asm volatile("csrr %0, satp" : "=r"(tcb->trap_frame->kernel_satp));
    tcb->trap_frame->kernel_sp   = reinterpret_cast<uintptr_t>(tcb->kernel_trap_stack) + PAGE_SIZE;
    tcb->trap_frame->kernel_trap = reinterpret_cast<uintptr_t>(user_trap);

    sstatus &= ~(1 << 8);
    sstatus |= (1 << 5);
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw sepc, %0" : : "r"(tcb->trap_frame->epc));

    return_to_user_mode(tcb->satp);
  }
} // namespace caprese::arch
