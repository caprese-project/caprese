/**
 * @file linker.lds
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Linker script
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#define PAGE_SIZE 4096
#define VPN_SIZE (1 << 9)
#define VPN_MASK (VPN_SIZE - 1)
#define VPN_X2_SIZE (1 << 18)
#define VPN_X2_MASK (VPN_X2_SIZE - 1)

#define ROUND_UP(value, align) (((value) + (align) - 1) / (align) * (align))
#define ROUND_DOWN(value, align) ((value) / (align) * (align))

#define GET_NUM_PTE2(size_of_vas) ((ROUND_UP((size_of_vas), 1 << 30) >> 30) & VPN_MASK)
#define GET_NUM_PTE1(size_of_vas) ((VPN_SIZE * ((ROUND_DOWN((size_of_vas), 1 << 30) >> 30) & VPN_MASK)) + ((ROUND_UP((size_of_vas), 1 << 21) >> 21) & VPN_MASK))
#define GET_NUM_PTE0(size_of_vas) ((VPN_SIZE * ((ROUND_DOWN((size_of_vas), 1 << 21) >> 21) & VPN_X2_MASK)) + ((ROUND_UP((size_of_vas), 1 << 12) >> 12) & VPN_MASK))
#define GET_BASE_PTE2(base_pte) (base_pte)
#define GET_BASE_PTE1(base_pte2, num_pte2) ((base_pte2) + ROUND_UP((num_pte2), PAGE_SIZE))
#define GET_BASE_PTE0(base_pte1, num_pte1) ((base_pte1) + ROUND_UP((num_pte1), PAGE_SIZE))

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x80000000;

  begin_of_kernel = .;

  . = 0x80200000;

  begin_of_kernel_virtual_address_space = .;

  begin_of_kernel_text_section = .;
  .text :
  {
    *(.text.start)
    *(.text)
    . = ALIGN(PAGE_SIZE);
    *(.text.trampoline)
  }
  . = ALIGN(16);
  end_of_kernel_text_section = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_kernel_rodata_section = .;
  .rodata :
  {
    *(.srodata*)
    . = ALIGN(16);
    *(.rodata)
    . = ALIGN(16);
    *(.rodata.*)
  }
  . = ALIGN(16);
  end_of_kernel_rodata_section = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_kernel_data_section = .;
  .data :
  {
    __global_pointer$ = . + 0x800;
    *(.sdata*)
    *(.data)
    *(.data.*)
  }
  . = ALIGN(16);
  end_of_kernel_data_section = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_kernel_bss_section = .;
  .bss :
  {
    *(.sbss*)
    *(.bss)
    *(.bss.*)
  }
  . = ALIGN(16);
  end_of_kernel_bss_section = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_kernel_stack = .;
  .stack :
  {
    *(.stack)
  }
  . = ALIGN(16);
  end_of_kernel_stack = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_kernel_page_table = .;

  #define K_SIZE_OF_VAS (end_of_kernel_virtual_address_space - begin_of_kernel_virtual_address_space)
  #define K_NUM_PTE2 GET_NUM_PTE2(K_SIZE_OF_VAS)
  #define K_NUM_PTE1 GET_NUM_PTE1(K_SIZE_OF_VAS)
  #define K_NUM_PTE0 GET_NUM_PTE0(K_SIZE_OF_VAS)
  #define K_BASE_PTE2 GET_BASE_PTE2(begin_of_kernel_page_table)
  #define K_BASE_PTE1 GET_BASE_PTE1(K_BASE_PTE2, K_NUM_PTE2)
  #define K_BASE_PTE0 GET_BASE_PTE0(K_BASE_PTE1, K_NUM_PTE1)

  . = K_BASE_PTE0 + ROUND_UP(K_NUM_PTE0, PAGE_SIZE) + PAGE_SIZE;

  . = ALIGN(PAGE_SIZE);

  end_of_kernel_page_table = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_root_server_page_table = .;

  #define RS_SIZE_OF_VAS (end_of_root_server - begin_of_root_server)
  #define RS_NUM_PTE2 GET_NUM_PTE2(RS_SIZE_OF_VAS)
  #define RS_NUM_PTE1 GET_NUM_PTE1(RS_SIZE_OF_VAS)
  #define RS_NUM_PTE0 GET_NUM_PTE0(RS_SIZE_OF_VAS)
  #define RS_BASE_PTE2 GET_BASE_PTE2(begin_of_root_server_page_table)
  #define RS_BASE_PTE1 GET_BASE_PTE1(RS_BASE_PTE2, RS_NUM_PTE2)
  #define RS_BASE_PTE0 GET_BASE_PTE0(RS_BASE_PTE1, RS_NUM_PTE1)

  . = RS_BASE_PTE0 + ROUND_UP(RS_NUM_PTE0, PAGE_SIZE) + PAGE_SIZE;

  . = ALIGN(PAGE_SIZE);

  end_of_root_server_page_table = .;

  end_of_kernel_virtual_address_space = .;

  . = ALIGN(PAGE_SIZE);

  begin_of_root_server_virtual_address_space = .;

  .root_server :
  {
    begin_of_root_server = .;
    *(.root_server)
    end_of_root_server = .;
    . = ALIGN(PAGE_SIZE);
    begin_of_root_server_trap_frame = .;
    . = . + PAGE_SIZE;
    end_of_root_server_trap_frame = .;
  }

  end_of_root_server_virtual_address_space = .;

  . = ALIGN(16);
  end_of_kernel = .;
}
