/**
 * @file boot.cpp
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

#include <algorithm>
#include <cstdio>

#include <caprese/arch/rv64/trap/trap.h>
#include <caprese/capability/capability.h>
#include <caprese/cpu/cpu_info.h>
#include <caprese/main.h>
#include <caprese/memory/heap.h>
#include <caprese/memory/page_stack.h>
#include <caprese/panic.h>
#include <caprese/task/task.h>

#include <boot_loader/rv64/include/boot_info.h>
#include <libcaprese/util/enum.h>

extern "C" {
  extern char _begin_of_root_server[];
  extern char _end_of_root_server[];
}

namespace caprese::arch {
  namespace {
    void init_trap() {
      asm volatile("csrw stvec, %0" : : "r"(kernel_vector));
    }

    void init_memory(boot_loader::boot_info_t* info) {
      size_t initial_memory_size = std::min<size_t>(2 * 1024 * 1024, info->total_memory_size >> 7);
      size_t pushed_memory_size  = 0;

      auto cap_table = capability::get_capability_table_by_id(1);
      while (pushed_memory_size < initial_memory_size) {
        for (uint8_t i = 0; i < cap_table->used; ++i) {
          if (cap_table->slots[i].type() == capability::capability_type::memory && cap_table->slots[i].memory.device == 0) {
            uintptr_t addr = caprese::memory::virt_to_phys(cap_table->slots[i].memory.address()).value();

            for (size_t j = 0; info->used_regions[j].end != 0; ++j) {
              if (info->used_regions[j].begin <= addr && addr < info->used_regions[j].end) {
                goto for_end;
              }
            }

            if (pushed_memory_size == 0) [[unlikely]] {
              caprese::memory::append_heap(capability::make_handle(&cap_table->slots[i]));
            } else {
              caprese::memory::push_page(capability::make_handle(&cap_table->slots[i]));
            }

            pushed_memory_size += caprese::memory::page_size();
            if (pushed_memory_size >= initial_memory_size) [[unlikely]] {
              break;
            }
          }
        for_end:
          continue;
        }
        if (cap_table->next_cap_space_id == 0) [[unlikely]] {
          break;
        }
        cap_table = capability::get_capability_table_by_id(cap_table->next_cap_space_id);
      }

      cap_table = capability::get_capability_table_by_id(1);
      while (true) {
        for (uint8_t i = 0; i < cap_table->used; ++i) {
          if (cap_table->slots[i].type() == capability::capability_type::memory && cap_table->slots[i].memory.device == 0) {
            uintptr_t addr = caprese::memory::virt_to_phys(cap_table->slots[i].memory.address()).value();
            if (info->used_regions[0].begin <= addr && addr < info->used_regions[0].end) [[unlikely]] {
              memory::unmap(info->root_page_table, addr);
            }
          }
        }
        if (cap_table->next_cap_space_id == 0) [[unlikely]] {
          break;
        }
        cap_table = capability::get_capability_table_by_id(cap_table->next_cap_space_id);
      }
    }

    void init_root_task() {
      auto root_task_id = caprese::task::create_new_task();
      auto root_task    = caprese::task::get_task_by_id(root_task_id);

      size_t size = _end_of_root_server - _begin_of_root_server;
      auto   base = caprese::memory::walk_page(root_task->arch_task.get_root_page_table(), _begin_of_root_server);
      for (size_t page = 0; page < size; page += caprese::memory::page_size()) {
        caprese::memory::map(root_task->arch_task.get_root_page_table(),
                             CONFIG_ROOT_SERVER_BASE_ADDRESS + page,
                             base.value() + page,
                             caprese::memory::page_flag::readable | caprese::memory::page_flag::writable | caprese::memory::page_flag::executable
                                 | caprese::memory::page_flag::user_mode);
      }

      root_task->arch_task.setup(CONFIG_ROOT_SERVER_BASE_ADDRESS);
    }
  } // namespace

  void boot_kernel_primary(boot_loader::boot_info_t* info) {
    printf("\nBooting the kernel...\n");

    init_trap();
    init_memory(info);
    caprese::task::init_task_space(info->root_page_table);
    caprese::cpu::setup(info->hartid, info->nhart);
    caprese::task::set_current_task(0);
    init_root_task();
  }

  void boot_kernel_secondary() { }
} // namespace caprese::arch

extern "C" [[noreturn]] void arch_main() {
  caprese::boot_loader::boot_info_t* info;

  asm volatile("mv %0, a0" : "=r"(info));

  if (info->hartid == 0) {
    caprese::arch::boot_kernel_primary(info);
  } else {
    caprese::panic("Multicore is not yet supported. hartid: %d", info->hartid);
    caprese::arch::boot_kernel_secondary();
  }

  caprese::main();
}
