/**
 * @file arch_main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief C entry point
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <cstdint>
#include <utility>

#include <caprese/kernel/main.h>
#include <caprese/lib/console.h>

#include <caprese/arch/riscv/kernel_layout.h>
#include <caprese/arch/riscv/page_table.h>
#include <caprese/arch/riscv/thread_control_block.h>
#include <caprese/arch/riscv/trap.h>

namespace {
  constexpr const auto TAG = "boot";
  using namespace caprese;
  using namespace caprese::arch;

  const page_region_info kernel_page_region_infos[] = {
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_text_section),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_text_section),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_text_section),
     .readable                  = true,
     .writable                  = false,
     .executable                = true,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_rodata_section),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_rodata_section),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_rodata_section),
     .readable                  = true,
     .writable                  = false,
     .executable                = false,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_data_section),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_data_section),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_data_section),
     .readable                  = true,
     .writable                  = true,
     .executable                = false,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_bss_section),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_bss_section),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_bss_section),
     .readable                  = true,
     .writable                  = true,
     .executable                = false,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_stack),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_stack),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_stack),
     .readable                  = true,
     .writable                  = true,
     .executable                = false,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_kernel_page_table),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_kernel_page_table),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_kernel_page_table),
     .readable                  = true,
     .writable                  = true,
     .executable                = false,
     .user                      = false,
     },
    {
     .base_virtual_address      = reinterpret_cast<virtual_address_t>(&begin_of_root_server_page_table),
     .begin_of_physical_address = reinterpret_cast<physical_address_t>(&begin_of_root_server_page_table),
     .end_of_physical_address   = reinterpret_cast<physical_address_t>(&end_of_root_server_page_table),
     .readable                  = true,
     .writable                  = true,
     .executable                = false,
     .user                      = false,
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

extern "C" [[noreturn]] void arch_main(uintptr_t hartid, const void* dtb) {
  using namespace caprese;
  using namespace caprese::arch;

  log_assert(TAG, hartid == 0, "hart protection: %d", hartid);

  log_info(TAG, "Kernel is booting on hartid %u...", hartid);
  log_debug(TAG, "DTB installation address: %p", dtb);

  log_debug(TAG, "text section:           %p to %p", &begin_of_kernel_text_section, &end_of_kernel_text_section);
  log_debug(TAG, "rodata section:         %p to %p", &begin_of_kernel_rodata_section, &end_of_kernel_rodata_section);
  log_debug(TAG, "data section:           %p to %p", &begin_of_kernel_data_section, &end_of_kernel_data_section);
  log_debug(TAG, "bss section:            %p to %p", &begin_of_kernel_bss_section, &end_of_kernel_bss_section);
  log_debug(TAG, "kernel stack:           %p to %p", &begin_of_kernel_stack, &end_of_kernel_stack);
  log_debug(TAG, "root server:            %p to %p", &begin_of_root_server, &end_of_root_server);
  log_debug(TAG, "kernel page table:      %p to %p", &begin_of_kernel_page_table, &end_of_kernel_page_table);
  log_debug(TAG, "root server page table: %p to %p", &begin_of_root_server_page_table, &end_of_root_server_page_table);

  caprese::error_t error;

  const auto kernel_page_table_range =
      std::make_pair(reinterpret_cast<physical_address_t>(&begin_of_kernel_page_table), reinterpret_cast<physical_address_t>(&end_of_kernel_page_table));

  early_page_table kernel_page_table;
  if (failed(error = kernel_page_table.create(kernel_page_table_range.first, kernel_page_table_range.second, kernel_page_region_infos))) [[unlikely]] {
    log_fatal(TAG, "Failed to create kernel page table. code: %d", error);
  }

  if (failed(error = kernel_page_table.enable_mmu())) [[unlikely]] {
    log_fatal(TAG, "Failed to enable MMU. code: %d", error);
  }

  auto root_tcb = thread_control_block::current();
  log_info(TAG, "Jump to root server...");

  thread_context empty_context {};
  switch_context(&empty_context, &root_tcb->context);

  log_debug(TAG, "unreachable");

  main();
}
