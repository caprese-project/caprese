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

#ifndef CAPRESE_ARCH_RV64_TASK_TASK_H_
#define CAPRESE_ARCH_RV64_TASK_TASK_H_

#include <cstdint>

#include <caprese/memory/address.h>
#include <caprese/memory/page.h>

namespace caprese::arch::task {
  struct task_context;
} // namespace caprese::arch::task

extern "C" void switch_context(const caprese::arch::task::task_context* old_context, const caprese::arch::task::task_context* new_context);

namespace caprese::arch::task {
  struct task_context {
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

  struct task_trap_frame {
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

  struct task_t {
    task_trap_frame trap_frame;
    task_context    context;
    uintptr_t       satp;
    alignas(16) char trampoline[0];

    inline void set_root_page_table(caprese::memory::virtual_address_t root_page_table) {
      satp = (9ull << 60) | (caprese::memory::virt_to_phys(root_page_table).value() >> caprese::memory::page_size_bit());
    }

    inline caprese::memory::virtual_address_t get_root_page_table() {
      return caprese::memory::phys_to_virt((satp & ((1ull << 44) - 1)) << caprese::memory::page_size_bit());
    }

    void setup(caprese::memory::virtual_address_t entry_address);
  };

  inline void switch_context(task_t* old_task, task_t* new_task) {
    ::switch_context(&old_task->context, &new_task->context);
  }
} // namespace caprese::arch::task

#endif // CAPRESE_ARCH_RV64_TASK_TASK_H_
