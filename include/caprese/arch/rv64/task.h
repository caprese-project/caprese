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
    uintptr_t sepc;
    uintptr_t satp;
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

  struct task_t {
    context_t    context;
    trap_frame_t trap_frame;
  };

  void      create_kernel_task(task_t* task, void (*entry)(const boot_info_t*), const boot_info_t* boot_info);
  void      load_init_task_payload(task_t* init_task, const arch::boot_info_t* boot_info);
  void      init_task(task_t* task, uintptr_t stack_address);
  void      switch_context(task_t* old_task, task_t* new_task);
  void      load_context(task_t* task);
  uintptr_t get_root_page_table(task_t* task);
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_TASK_H_
