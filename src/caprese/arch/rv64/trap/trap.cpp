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
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstdio>

#include <caprese/arch/common/trap/trap.h>
#include <caprese/arch/rv64/task/task.h>
#include <caprese/arch/rv64/trap/system_call.h>
#include <caprese/task/task.h>

extern "C" {
  void kernel_trap() {
    uintptr_t sepc, sstatus, scause;
    asm volatile("csrr %0, sepc" : "=r"(sepc));
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    asm volatile("csrr %0, scause" : "=r"(scause));

    printf("kernel_trap: sepc=%lx, sstatus=%lx, scause=%lx\n", sepc, sstatus, scause);
    // TODO: handle trap

    asm volatile("csrw sepc, %0" : : "r"(sepc));
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
  }
}

namespace caprese::arch::trap {
  namespace {
    void user_trap() {
      asm volatile("csrw stvec, %0" : : "r"(kernel_vector));

      auto task = caprese::task::get_current_task();
      asm volatile("csrr %0, sepc" : "=r"(task->arch_task.trap_frame.epc));

      riscv_trap_code scause;
      asm volatile("csrr %0, scause" : "=r"(scause));

      // TODO: handle trap
      switch (scause) {
        using enum riscv_trap_code;
        case INSTRUCTION_ADDRESS_MISALIGNED:
          printf("INSTRUCTION_ADDRESS_MISALIGNED\n");
          break;
        case INSTRUCTION_ACCESS_FAULT:
          printf("INSTRUCTION_ACCESS_FAULT\n");
          break;
        case ILLEGAL_INSTRUCTION:
          printf("ILLEGAL_INSTRUCTION\n");
          break;
        case BREAKPOINT:
          printf("BREAKPOINT\n");
          break;
        case LOAD_ADDRESS_MISALIGNED:
          printf("LOAD_ADDRESS_MISALIGNED\n");
          break;
        case LOAD_ACCESS_FAULT:
          printf("LOAD_ACCESS_FAULT\n");
          break;
        case STORE_AMO_ADDRESS_MISALIGNED:
          printf("STORE_AMO_ADDRESS_MISALIGNED\n");
          break;
        case STORE_AMO_ACCESS_FAULT:
          printf("STORE_AMO_ACCESS_FAULT\n");
          break;
        case ENVIRONMENT_CALL_FROM_U_MODE:
          handle_system_call();
          task->arch_task.trap_frame.epc += 4;
          break;
        case ENVIRONMENT_CALL_FROM_S_MODE:
          printf("ENVIRONMENT_CALL_FROM_S_MODE\n");
          break;
        case ENVIRONMENT_CALL_FROM_M_MODE:
          printf("ENVIRONMENT_CALL_FROM_M_MODE\n");
          break;
        case INSTRUCTION_PAGE_FAULT:
          printf("INSTRUCTION_PAGE_FAULT\n");
          break;
        case LOAD_PAGE_FAULT:
          printf("LOAD_PAGE_FAULT\n");
          break;
        case STORE_AMO_PAGE_FAULT:
          printf("STORE_AMO_PAGE_FAULT\n");
          break;
        default:
          printf("Unknown scause: %p\n", reinterpret_cast<void*>(scause));
      }

      return_to_user_mode();
    }
  } // namespace

  void return_to_user_mode() {
    auto  task      = caprese::task::get_current_task();
    auto& arch_task = task->arch_task;

    const auto user_vector_offset         = reinterpret_cast<uintptr_t>(trampoline_user_vector) - reinterpret_cast<uintptr_t>(begin_of_trampoline);
    const auto return_to_user_mode_offset = reinterpret_cast<uintptr_t>(trampoline_return_to_user_mode) - reinterpret_cast<uintptr_t>(begin_of_trampoline);

    const auto user_vector         = reinterpret_cast<trampoline_user_vector_t>(arch_task.trampoline + user_vector_offset);
    const auto return_to_user_mode = reinterpret_cast<trampoline_return_to_user_mode_t>(arch_task.trampoline + return_to_user_mode_offset);

    uintptr_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw stvec, %0" : : "r"(user_vector));
    asm volatile("csrr %0, satp" : "=r"(arch_task.trap_frame.kernel_satp));
    arch_task.trap_frame.kernel_sp   = reinterpret_cast<uintptr_t>(task->kernel_stack) + caprese::memory::page_size();
    arch_task.trap_frame.kernel_trap = reinterpret_cast<uintptr_t>(user_trap);

    sstatus &= ~(1 << 8);
    sstatus |= (1 << 5);
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    asm volatile("csrw sepc, %0" : : "r"(arch_task.trap_frame.epc));

    return_to_user_mode(&arch_task.trap_frame, arch_task.satp);
  }
} // namespace caprese::arch::trap
