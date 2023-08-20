/**
 * @file task.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#ifndef CAPRESE_ARCH_RV64_TASK_H_
#define CAPRESE_ARCH_RV64_TASK_H_

#include <cstddef>
#include <cstdint>

#include <caprese/arch/boot_info.h>

namespace caprese::arch::inline rv64 {
  struct context_t {
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

  struct trap_frame_t {
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
    uintptr_t sepc;
    uintptr_t satp;
    uintptr_t hartid;
  };

  constexpr uintptr_t REGISTER_RA   = offsetof(trap_frame_t, ra);
  constexpr uintptr_t REGISTER_SP   = offsetof(trap_frame_t, sp);
  constexpr uintptr_t REGISTER_GP   = offsetof(trap_frame_t, gp);
  constexpr uintptr_t REGISTER_TP   = offsetof(trap_frame_t, tp);
  constexpr uintptr_t REGISTER_T0   = offsetof(trap_frame_t, t0);
  constexpr uintptr_t REGISTER_T1   = offsetof(trap_frame_t, t1);
  constexpr uintptr_t REGISTER_T2   = offsetof(trap_frame_t, t2);
  constexpr uintptr_t REGISTER_S0   = offsetof(trap_frame_t, s0);
  constexpr uintptr_t REGISTER_S1   = offsetof(trap_frame_t, s1);
  constexpr uintptr_t REGISTER_A0   = offsetof(trap_frame_t, a0);
  constexpr uintptr_t REGISTER_A1   = offsetof(trap_frame_t, a1);
  constexpr uintptr_t REGISTER_A2   = offsetof(trap_frame_t, a2);
  constexpr uintptr_t REGISTER_A3   = offsetof(trap_frame_t, a3);
  constexpr uintptr_t REGISTER_A4   = offsetof(trap_frame_t, a4);
  constexpr uintptr_t REGISTER_A5   = offsetof(trap_frame_t, a5);
  constexpr uintptr_t REGISTER_A6   = offsetof(trap_frame_t, a6);
  constexpr uintptr_t REGISTER_A7   = offsetof(trap_frame_t, a7);
  constexpr uintptr_t REGISTER_S2   = offsetof(trap_frame_t, s2);
  constexpr uintptr_t REGISTER_S3   = offsetof(trap_frame_t, s3);
  constexpr uintptr_t REGISTER_S4   = offsetof(trap_frame_t, s4);
  constexpr uintptr_t REGISTER_S5   = offsetof(trap_frame_t, s5);
  constexpr uintptr_t REGISTER_S6   = offsetof(trap_frame_t, s6);
  constexpr uintptr_t REGISTER_S7   = offsetof(trap_frame_t, s7);
  constexpr uintptr_t REGISTER_S8   = offsetof(trap_frame_t, s8);
  constexpr uintptr_t REGISTER_S9   = offsetof(trap_frame_t, s9);
  constexpr uintptr_t REGISTER_S10  = offsetof(trap_frame_t, s10);
  constexpr uintptr_t REGISTER_S11  = offsetof(trap_frame_t, s11);
  constexpr uintptr_t REGISTER_T3   = offsetof(trap_frame_t, t3);
  constexpr uintptr_t REGISTER_T4   = offsetof(trap_frame_t, t4);
  constexpr uintptr_t REGISTER_T5   = offsetof(trap_frame_t, t5);
  constexpr uintptr_t REGISTER_T6   = offsetof(trap_frame_t, t6);
  constexpr uintptr_t REGISTER_SEPC = offsetof(trap_frame_t, sepc);
  constexpr uintptr_t REGISTER_SATP = offsetof(trap_frame_t, satp);

  constexpr uintptr_t ABI_RETURN_ADDRESS  = REGISTER_RA;
  constexpr uintptr_t ABI_STACK_POINTER   = REGISTER_SP;
  constexpr uintptr_t ABI_GLOBAL_POINTER  = REGISTER_GP;
  constexpr uintptr_t ABI_THREAD_POINTER  = REGISTER_TP;
  constexpr uintptr_t ABI_PROGRAM_COUNTER = REGISTER_SEPC;

  constexpr uintptr_t ABI_ARGUMENTS[6] = {
    REGISTER_A0, REGISTER_A1, REGISTER_A2, REGISTER_A3, REGISTER_A4, REGISTER_A5,
  };
  constexpr uintptr_t ABI_RETURNS[2] = {
    REGISTER_A0,
    REGISTER_A1,
  };
  constexpr uintptr_t ABI_CALLEE_SAVES[13] = {
    REGISTER_SP, REGISTER_S0, REGISTER_S1, REGISTER_S2, REGISTER_S3, REGISTER_S4, REGISTER_S5, REGISTER_S6, REGISTER_S7, REGISTER_S8, REGISTER_S9, REGISTER_S10, REGISTER_S11,
  };
  constexpr uintptr_t ABI_CALLER_SAVES[16] = {
    REGISTER_RA, REGISTER_T0, REGISTER_T1, REGISTER_T2, REGISTER_T3, REGISTER_T4, REGISTER_T5, REGISTER_T6,
    REGISTER_A0, REGISTER_A1, REGISTER_A2, REGISTER_A3, REGISTER_A4, REGISTER_A5, REGISTER_A6, REGISTER_A7,
  };

  struct task_t {
    context_t    context;
    trap_frame_t trap_frame;
  };

  static_assert(offsetof(task_t, context) == CONFIG_ARCH_TASK_CONTEXT_OFFSET);
  static_assert(offsetof(task_t, trap_frame) == CONFIG_ARCH_TASK_TRAP_FRAME_OFFSET);

  [[nodiscard]] bool      create_kernel_task(task_t* task, void (*entry)(const boot_info_t*), const boot_info_t* boot_info);
  [[nodiscard]] bool      load_init_task_payload(uint32_t init_task_cid_handle, const arch::boot_info_t* boot_info);
  [[nodiscard]] bool      init_task(task_t* task, uintptr_t stack_address);
  void                    switch_context(task_t* old_task, task_t* new_task);
  void                    load_context(task_t* task);
  void                    set_root_page_table(task_t* task, uintptr_t root_page_table);
  [[nodiscard]] uintptr_t get_root_page_table(task_t* task);
  void                    set_register(task_t* task, uintptr_t reg, uintptr_t value);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_TASK_H_
