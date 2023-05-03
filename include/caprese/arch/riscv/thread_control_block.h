/**
 * @file thread_control_block.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declares functions related to the thread control block.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RISCV_THREAD_CONTROL_BLOCK_H_
#define CAPRESE_ARCH_RISCV_THREAD_CONTROL_BLOCK_H_

#include <cstddef>
#include <cstdint>

namespace caprese::arch {
  struct thread_context {
    uintptr_t ra;
    uintptr_t sp;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;
  };

  struct thread_trap_frame {
    uintptr_t kernel_satp;
    uintptr_t kernel_sp;
    uintptr_t kernel_trap;
    uintptr_t epc;
    uintptr_t kernel_hartid;
    uintptr_t ra;
    uintptr_t sp;
    uintptr_t gp;
    uintptr_t tp;
    uintptr_t t0;
    uintptr_t t1;
    uintptr_t t2;
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t a0;
    uintptr_t a1;
    uintptr_t a2;
    uintptr_t a3;
    uintptr_t a4;
    uintptr_t a5;
    uintptr_t a6;
    uintptr_t a7;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;
    uintptr_t t3;
    uintptr_t t4;
    uintptr_t t5;
    uintptr_t t6;
  };

  struct thread_control_block {
    const size_t       tcb_id;
    thread_context     context;
    thread_trap_frame* trap_frame;
    uintptr_t          satp;
    void*              kernel_trap_stack;

  public:
    static thread_control_block* current();
  };

  extern thread_control_block root_server_thread_control_block;
} // namespace caprese::arch

extern "C" void switch_context(const caprese::arch::thread_context* old_context, const caprese::arch::thread_context* new_context);

#endif // CAPRESE_ARCH_RISCV_THREAD_CONTROL_BLOCK_H_
