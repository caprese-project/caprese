/**
 * @file page_table.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Defines functions related to page tables.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#include <caprese/lib/console.h>

#include <caprese/arch/riscv/page_table.h>

namespace caprese::arch {
  error_t early_page_table::enable_mmu() {
    asm volatile("sfence.vma zero, zero");
    asm volatile("csrw satp, %0" : : "r"(get_satp_value()));
    asm volatile("sfence.vma zero, zero");

    log_info("early_page_table", "MMU enabled.");

    return error_t::OK;
  }

  uintptr_t early_page_table::get_satp_value() {
    uintptr_t mode = 8ull << 60;
    uintptr_t ppn  = reinterpret_cast<uintptr_t>(satp_page_table_entry) >> 12;
    return mode | ppn;
  }
} // namespace caprese::arch
