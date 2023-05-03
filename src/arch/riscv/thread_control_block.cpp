/**
 * @file thread_control_block.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Defines functions related to the thread control block.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#include <caprese/arch/riscv/kernel_layout.h>
#include <caprese/arch/riscv/page_table.h>
#include <caprese/arch/riscv/thread_control_block.h>
#include <caprese/arch/riscv/trap.h>

namespace caprese::arch {
  namespace {
    const page_region_info root_server_page_region_infos[] = {
      {
       .base_virtual_address      = CONFIG_ROOT_SERVER_BASE_ADDRESS,
       .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_root_server),
       .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_root_server),
       .readable                  = true,
       .writable                  = true,
       .executable                = true,
       .user                      = true,
       },
      {
       .base_virtual_address      = trap_frame_base_address,
       .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_root_server_trap_frame),
       .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_root_server_trap_frame),
       .readable                  = true,
       .writable                  = true,
       .executable                = false,
       .user                      = false,
       },
      {
       .base_virtual_address      = trampoline_base_address,
       .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_trampoline),
       .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_trampoline),
       .readable                  = true,
       .writable                  = false,
       .executable                = true,
       .user                      = false,
       },
    };
  } // namespace

  thread_control_block root_server_thread_control_block = {};

  bool inited = false;
  char root_server_kernel_trap_stack[page_size];

  thread_control_block* thread_control_block::current() {
    // TODO: impl

    if (!inited) {
      early_page_table page_table;
      if (failed(page_table.create(reinterpret_cast<physical_address_t>(&begin_of_root_server_page_table),
                                   reinterpret_cast<physical_address_t>(&end_of_root_server_page_table),
                                   root_server_page_region_infos))) [[unlikely]] {
        log_fatal("thread_control_block", "Failed to create root server's page table.");
      }
      root_server_thread_control_block.satp              = page_table.get_satp_value();
      root_server_thread_control_block.kernel_trap_stack = root_server_kernel_trap_stack;
      root_server_thread_control_block.context.ra        = reinterpret_cast<uintptr_t>(return_to_user_mode);
      root_server_thread_control_block.context.sp        = reinterpret_cast<uintptr_t>(root_server_thread_control_block.kernel_trap_stack) + page_size;
      root_server_thread_control_block.trap_frame        = reinterpret_cast<thread_trap_frame*>(trap_frame_base_address);
      root_server_thread_control_block.trap_frame->epc   = CONFIG_ROOT_SERVER_BASE_ADDRESS;
      inited                                             = true;
    }

    return &root_server_thread_control_block;
  }
} // namespace caprese::arch
