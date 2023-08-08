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

#include <caprese/arch/memory.h>
#include <caprese/arch/rv64/csr.h>
#include <caprese/arch/rv64/task.h>
#include <caprese/arch/system.h>
#include <caprese/memory/address.h>
#include <caprese/memory/kernel_space.h>
#include <caprese/task/task.h>

extern "C" {
  extern void _jump_to_kernel_entry();
  extern void _switch_context(caprese::arch::task_t* old_task, caprese::arch::task_t* new_task);
  extern void _load_context([[maybe_unused]] nullptr_t, caprese::arch::task_t* task);

  extern const char _payload_start[];
  extern const char _payload_end[];
  extern const char _stack[];
}

namespace caprese::arch::inline rv64 {
  namespace {
    constexpr size_t PAGE_SIZE_BIT = std::countr_zero(PAGE_SIZE);

    void return_to_user_mode() {
      printf("Return To User Mode!\n");
    };
  } // namespace

  void create_kernel_task(task_t* task, void (*entry)(const boot_info_t*), const boot_info_t* boot_info) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(_jump_to_kernel_entry);
    task->context.sp = CONFIG_STACK_SPACE_BASE + arch::PAGE_SIZE;
    task->context.s0 = reinterpret_cast<uint64_t>(entry);
    task->context.s1 = reinterpret_cast<uint64_t>(boot_info);

    asm volatile("csrr %0, satp" : "=r"(task->trap_frame.satp));

    uintptr_t root_page_table = memory::physical_address_t::from(get_root_page_table(task)).mapped_address().value;
    map_page(root_page_table,
             CONFIG_STACK_SPACE_BASE,
             memory::mapped_address_t::from(_stack).physical_address().value - arch::PAGE_SIZE,
             { .readable = true, .writable = true, .executable = false, .user = false },
             true);
  }

  void load_init_task_payload(task_t* init_task, const arch::boot_info_t* boot_info) {
    init_task->trap_frame.a0   = get_core_id();
    init_task->trap_frame.a1   = reinterpret_cast<uint64_t>(boot_info);
    init_task->trap_frame.sepc = CONFIG_USER_PAYLOAD_BASE_ADDRESS;

    uintptr_t start           = reinterpret_cast<uintptr_t>(_payload_start);
    uintptr_t end             = reinterpret_cast<uintptr_t>(_payload_end);
    uintptr_t payload_size    = end - start;
    uintptr_t root_page_table = memory::physical_address_t::from(get_root_page_table(init_task)).mapped_address().value;
    for (uintptr_t page = 0; page < payload_size; page += PAGE_SIZE) {
      map_page(root_page_table,
               CONFIG_USER_PAYLOAD_BASE_ADDRESS + page,
               memory::mapped_address_t::from(start + page).physical_address().value,
               { .readable = true, .writable = true, .executable = true, .user = true },
               true);
    }
  }

  void init_task(task_t* task, uintptr_t stack_address) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(return_to_user_mode);
    task->context.sp = stack_address + arch::PAGE_SIZE;

    auto root_page_table = memory::mapped_address_t::from(aligned_alloc(PAGE_SIZE, PAGE_SIZE));
    memset(root_page_table.as<void>(), 0, PAGE_SIZE);

    memory::copy_kernel_space_mapping(root_page_table, task::get_kernel_root_page_table());

    memory::mapped_address_t stack = memory::mapped_address_t::from(aligned_alloc(PAGE_SIZE, PAGE_SIZE));
    map_page(root_page_table.value,
             stack_address,
             stack.physical_address().value,
             { .readable = true, .writable = true, .executable = false, .user = false },
             true);

#if defined(CONFIG_MMU_SV39)
    task->trap_frame.satp = SATP_MODE_SV39 | (root_page_table.physical_address().value >> PAGE_SIZE_BIT);
#elif defined(CONFIG_MMU_SV48)
    task->trap_frame.satp = SATP_MODE_SV48 | (root_page_table.physical_address().value >> PAGE_SIZE_BIT);
#endif
  }

  void switch_context(task_t* old_task, task_t* new_task) {
    _switch_context(old_task, new_task);
  }

  void load_context(task_t* task) {
    _load_context(nullptr, task);
  }

  uintptr_t get_root_page_table(task_t* task) {
    return (task->trap_frame.satp & SATP_PPN) << PAGE_SIZE_BIT;
  }
} // namespace caprese::arch::inline rv64
