/**
 * @file task.cpp
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

#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <caprese/arch/builtin_capability.h>
#include <caprese/arch/device.h>
#include <caprese/arch/memory.h>
#include <caprese/arch/rv64/csr.h>
#include <caprese/arch/rv64/task.h>
#include <caprese/arch/system.h>
#include <caprese/capability/bic/memory.h>
#include <caprese/capability/bic/task.h>
#include <caprese/memory/address.h>
#include <caprese/memory/kernel_space.h>
#include <caprese/syscall/handler.h>
#include <caprese/task/task.h>
#include <caprese/util/align.h>

namespace {
  void return_to_user_mode();
} // namespace

extern "C" {
  extern void _jump_to_kernel_entry();
  extern void _return_to_user_mode(caprese::arch::trap_frame_t* trap_frame);
  extern void _switch_context(caprese::arch::task_t* old_task, caprese::arch::task_t* new_task);
  extern void _load_context([[maybe_unused]] nullptr_t, caprese::arch::task_t* task);

  extern const char _payload_start[];
  extern const char _payload_end[];
  extern const char _stack[];

  void _user_trap() {
    using namespace caprese;
    using namespace caprese::arch;

    uint64_t scause;
    asm volatile("csrr %0, scause" : "=r"(scause));

    task_t* task = &task::get_current_task()->arch_task;

    if (scause & SCAUSE_INTERRUPT) {
      printf("scause-interrupt: 0x%lx\n", scause & SCAUSE_EXCEPTION_CODE);
    } else {
      if (scause & SCAUSE_ENVIRONMENT_CALL_FROM_U_MODE) {
        uint64_t          code   = task->trap_frame.a7;
        uint64_t          arg0   = task->trap_frame.a0;
        uint64_t          arg1   = task->trap_frame.a1;
        uint64_t          arg2   = task->trap_frame.a2;
        uint64_t          arg3   = task->trap_frame.a3;
        uint64_t          arg4   = task->trap_frame.a4;
        uint64_t          arg5   = task->trap_frame.a5;
        syscall::sysret_t result = syscall::handle_system_call(code, arg0, arg1, arg2, arg3, arg4, arg5);

        task->trap_frame.a0 = result.result;
        task->trap_frame.a1 = result.error;
        task->trap_frame.sepc += 4;
      } else {
        printf("scause-exception: 0x%lx\n", scause & SCAUSE_EXCEPTION_CODE);
        panic("TRAP");
      }
    }

    return_to_user_mode();
  }
}

namespace {
  constexpr size_t PAGE_SIZE_BIT = std::countr_zero(caprese::arch::PAGE_SIZE);

  void return_to_user_mode() {
    using namespace caprese;
    using namespace caprese::arch;

    task_t* task = &task::get_current_task()->arch_task;
    _return_to_user_mode(&task->trap_frame);
  };
} // namespace

namespace caprese::arch::inline rv64 {
  bool create_kernel_task(task_t* task, void (*entry)(const boot_info_t*), const boot_info_t* boot_info) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(_jump_to_kernel_entry);
    task->context.sp = CONFIG_STACK_SPACE_BASE + arch::PAGE_SIZE;
    task->context.s0 = reinterpret_cast<uint64_t>(entry);
    task->context.s1 = reinterpret_cast<uint64_t>(boot_info);

    asm volatile("csrr %0, satp" : "=r"(task->trap_frame.satp));

    uintptr_t root_page_table = memory::physical_address_t::from(get_root_page_table(task)).mapped_address().value;
    bool      result          = map_page(root_page_table,
                           CONFIG_STACK_SPACE_BASE,
                           memory::mapped_address_t::from(_stack).physical_address().value - arch::PAGE_SIZE,
                           { .readable = true, .writable = true, .executable = false, .user = false, .global = true },
                           true);

    return result;
  }

  bool load_init_task_payload(uint32_t init_task_cid_handle, const arch::boot_info_t* boot_info) {
    task::task_t* kernel_task = task::get_kernel_task();

    task::cid_t* init_task_cid = task::lookup_cid(kernel_task, init_task_cid_handle);
    if (init_task_cid == nullptr || init_task_cid->ccid != capability::bic::task::CCID) [[unlikely]] {
      return false;
    }

    capability::capability_t* init_task_cap = task::lookup_capability(kernel_task, *init_task_cid);
    if (init_task_cap == nullptr) [[unlikely]] {
      return false;
    }

    auto [init_task_tid, init_task_tid_error] = capability::get_field(init_task_cap, capability::bic::task::field::TID);
    if (init_task_tid_error) [[unlikely]] {
      return false;
    }

    task::task_t* init_task = task::lookup(std::bit_cast<task::tid_t>(static_cast<uint32_t>(init_task_tid)));
    if (init_task == nullptr) [[unlikely]] {
      return false;
    }

    uintptr_t start        = reinterpret_cast<uintptr_t>(_payload_start);
    uintptr_t end          = reinterpret_cast<uintptr_t>(_payload_end);
    uintptr_t payload_size = round_up(end - start, PAGE_SIZE);

    for (uintptr_t page = 0; page < payload_size; page += PAGE_SIZE) {
      memory::physical_address_t phys_page = memory::mapped_address_t::from(start + page).physical_address();
      capability::capability_t*  cap       = capability::bic::memory::create(phys_page,
                                                                      capability::bic::memory::constant::READABLE | capability::bic::memory::constant::WRITABLE
                                                                          | capability::bic::memory::constant::EXECUTABLE);
      if (cap == nullptr) [[unlikely]] {
        return false;
      }

      capability::cap_ret_t result = capability::call_method(cap,
                                                             capability::bic::memory::method::MAP,
                                                             init_task_cid_handle,
                                                             CONFIG_USER_PAYLOAD_BASE_ADDRESS + page,
                                                             capability::bic::memory::constant::READABLE | capability::bic::memory::constant::WRITABLE | capability::bic::memory::constant::EXECUTABLE);
      if (result.error) [[unlikely]] {
        return false;
      }

      if (task::insert_capability(init_task, cap) == 0) [[unlikely]] {
        return false;
      }
    }

    auto [dtb_base, dtb_size] = get_dtb_space(boot_info);
    for (uintptr_t page = 0; page < dtb_size; page += PAGE_SIZE) {
      memory::physical_address_t phys_page = memory::physical_address_t::from(dtb_base + page);
      capability::capability_t*  cap       = capability::bic::memory::create(phys_page,
                                                                      capability::bic::memory::constant::READABLE | capability::bic::memory::constant::WRITABLE
                                                                          | capability::bic::memory::constant::EXECUTABLE);
      if (cap == nullptr) [[unlikely]] {
        return false;
      }

      capability::cap_ret_t result = capability::call_method(cap,
                                                             capability::bic::memory::method::MAP,
                                                             init_task_cid_handle,
                                                             CONFIG_USER_PAYLOAD_BASE_ADDRESS + payload_size + page,
                                                             capability::bic::memory::constant::READABLE);
      if (result.error) [[unlikely]] {
        return false;
      }

      if (task::insert_capability(init_task, cap) == 0) [[unlikely]] {
        return false;
      }
    }

    capability::capability_t* init_task_cap_copy = capability::copy_capability(init_task_cap, capability::bic::task::permission::ALL);
    if (init_task_cap_copy == nullptr) [[unlikely]] {
      return false;
    }
    task::cid_handle_t init_task_cid_handle_copy = task::insert_capability(init_task, init_task_cap_copy);
    if (init_task_cid_handle_copy == 0) [[unlikely]] {
      return false;
    }

    init_task->arch_task.trap_frame.a0   = boot_info->hartid;
    init_task->arch_task.trap_frame.a1   = CONFIG_USER_PAYLOAD_BASE_ADDRESS + payload_size;
    init_task->arch_task.trap_frame.a2   = init_task_cid_handle_copy;
    init_task->arch_task.trap_frame.sepc = CONFIG_USER_PAYLOAD_BASE_ADDRESS;

    return true;
  }

  bool init_task(task_t* task, uintptr_t stack_address) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(return_to_user_mode);
    task->context.sp = stack_address + arch::PAGE_SIZE;

    return true;
  }

  void switch_context(task_t* old_task, task_t* new_task) {
    _switch_context(old_task, new_task);
  }

  void load_context(task_t* task) {
    _load_context(nullptr, task);
  }

  void set_root_page_table(task_t* task, uintptr_t root_page_table) {
#if defined(CONFIG_MMU_SV39)
    task->trap_frame.satp = SATP_MODE_SV39 | (root_page_table >> PAGE_SIZE_BIT);
#elif defined(CONFIG_MMU_SV48)
    task->trap_frame.satp = SATP_MODE_SV48 | (root_page_table >> PAGE_SIZE_BIT);
#endif
  }

  uintptr_t get_root_page_table(task_t* task) {
    return (task->trap_frame.satp & SATP_PPN) << PAGE_SIZE_BIT;
  }

  void set_register(task_t* task, uintptr_t reg, uintptr_t value) {
    if (reg > REGISTER_SATP) [[unlikely]] {
      return;
    }
    if (reg & (sizeof(uintptr_t) - 1)) [[unlikely]] {
      return;
    }
    reinterpret_cast<uintptr_t*>(&task->trap_frame)[reg >> std::countr_zero(sizeof(uintptr_t))] = value;
  }
} // namespace caprese::arch::inline rv64
