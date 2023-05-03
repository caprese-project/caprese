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
        case instruction_address_misaligned:
          log_info("trap", "instruction_address_misaligned, %p");
          break;
        case instruction_access_fault:
          log_info("trap", "instruction_access_fault");
          break;
        case illegal_instruction:
          log_info("trap", "illegal_instruction");
          break;
        case breakpoint:
          log_info("trap", "breakpoint");
          break;
        case load_address_misaligned:
          log_info("trap", "load_address_misaligned");
          break;
        case load_access_fault:
          log_info("trap", "load_access_fault");
          break;
        case store_amo_address_misaligned:
          log_info("trap", "store_amo_address_misaligned");
          break;
        case store_amo_access_fault:
          log_info("trap", "store_amo_access_fault");
          break;
        case environment_call_from_u_mode:
          {
            auto tcb = thread_control_block::current();
            print("%c", tcb->trap_frame->a0);
            tcb->trap_frame->epc += 4;
          }
          break;
        case environment_call_from_s_mode:
          log_info("trap", "environment_call_from_s_mode");
          break;
        case environment_call_from_m_mode:
          log_info("trap", "environment_call_from_m_mode");
          break;
        case instruction_page_fault:
          log_info("trap", "instruction_page_fault");
          break;
        case load_page_fault:
          log_info("trap", "load_page_fault");
          break;
        case store_amo_page_fault:
          log_info("trap", "store_amo_page_fault");
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
        trampoline_base_address + (reinterpret_cast<uintptr_t>(&trampoline_user_vector) - reinterpret_cast<uintptr_t>(&begin_of_trampoline)));
    const auto return_to_user_mode = reinterpret_cast<trampoline_return_to_user_mode_t>(
        trampoline_base_address + (reinterpret_cast<uintptr_t>(&trampoline_return_to_user_mode) - reinterpret_cast<uintptr_t>(&begin_of_trampoline)));

    uintptr_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw stvec, %0" : : "r"(user_vector));
    asm volatile("csrr %0, satp" : "=r"(tcb->trap_frame->kernel_satp));
    tcb->trap_frame->kernel_sp   = reinterpret_cast<uintptr_t>(tcb->kernel_trap_stack) + page_size;
    tcb->trap_frame->kernel_trap = reinterpret_cast<uintptr_t>(user_trap);

    sstatus &= ~(1 << 8);
    sstatus |= (1 << 5);
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw sepc, %0" : : "r"(tcb->trap_frame->epc));

    return_to_user_mode(tcb->satp);
  }
} // namespace caprese::arch
