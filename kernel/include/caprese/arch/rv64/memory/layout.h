/**
 * @file kernel_layout.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declares the address of each section of the kernel. The values are defined in linker.lds.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_KERNEL_ARCH_RISCV_KERNEL_LAYOUT_H_
#define CAPRESE_KERNEL_ARCH_RISCV_KERNEL_LAYOUT_H_

extern "C" {
  extern void* begin_of_kernel;
  extern void* end_of_kernel;

  extern void* begin_of_kernel_text_section;
  extern void* end_of_kernel_text_section;

  extern void* begin_of_kernel_rodata_section;
  extern void* end_of_kernel_rodata_section;

  extern void* begin_of_kernel_data_section;
  extern void* end_of_kernel_data_section;

  extern void* begin_of_kernel_bss_section;
  extern void* end_of_kernel_bss_section;

  extern void* begin_of_kernel_stack;
  extern void* end_of_kernel_stack;

  extern void* begin_of_kernel_virtual_address_space;
  extern void* end_of_kernel_virtual_address_space;

  extern void* begin_of_kernel_page_table;
  extern void* end_of_kernel_page_table;

  extern void* begin_of_root_server_page_table;
  extern void* end_of_root_server_page_table;

  extern void* begin_of_root_server_virtual_address_space;
  extern void* end_of_root_server_virtual_address_space;

  extern void* begin_of_root_server;
  extern void* end_of_root_server;

  extern void* begin_of_root_server_trap_frame;
  extern void* end_of_root_server_trap_frame;

  extern void* begin_of_trampoline_section;
  extern void* end_of_trampoline_section;
}

#endif // CAPRESE_KERNEL_ARCH_RISCV_KERNEL_LAYOUT_H_
