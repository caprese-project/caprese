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
#include <cstdlib>
#include <cstring>

#include <caprese/arch/memory.h>
#include <caprese/arch/rv64/csr.h>
#include <caprese/arch/rv64/task.h>
#include <caprese/arch/system.h>
#include <caprese/memory/address.h>

extern "C" {
  extern void _jump_to_kernel_entry();
  extern void _switch_context(caprese::arch::task_t* old_task, caprese::arch::task_t* new_task);
  extern void _load_context([[maybe_unused]] nullptr_t, caprese::arch::task_t* task);

  extern const char _payload_start[];
  extern const char _payload_end[];
}

namespace caprese::arch::inline rv64 {
  namespace {
    constexpr size_t PAGE_SIZE_BIT = std::countr_zero(PAGE_SIZE);
  } // namespace

  void create_kernel_task(task_t* task, void (*entry)(const boot_info_t*), const boot_info_t* boot_info) {
    task->context    = {};
    task->trap_frame = {};

    task->context.ra = reinterpret_cast<uint64_t>(_jump_to_kernel_entry);
    task->context.sp = reinterpret_cast<uint64_t>(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE)) + arch::PAGE_SIZE;
    task->context.s0 = reinterpret_cast<uint64_t>(entry);
    task->context.s1 = reinterpret_cast<uint64_t>(boot_info);

    asm volatile("csrr %0, satp" : "=r"(task->trap_frame.satp));
  }

  void load_init_task_payload(task_t* init_task, const arch::boot_info_t* boot_info) {
    init_task->context.ra = 0; // return to user mode
    init_task->context.sp = 0; // cls sp

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
               { .readable = 1, .writable = 1, .executable = 1, .user = 1 },
               true);
    }
  }

  void init_task(task_t* task) {
    task->context    = {};
    task->trap_frame = {};

    auto root_page_table = memory::mapped_address_t::from(aligned_alloc(PAGE_SIZE, PAGE_SIZE));
    memset(root_page_table.as<void>(), 0, PAGE_SIZE);

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
