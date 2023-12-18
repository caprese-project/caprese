#ifndef ARCH_RV64_KERNEL_BOOT_INFO_H_
#define ARCH_RV64_KERNEL_BOOT_INFO_H_

#include <cstdint>

#include <kernel/address.h>
#include <kernel/cap_space.h>
#include <kernel/core_id.h>
#include <kernel/page.h>
#include <kernel/task.h>
#include <libcaprese/root_boot_info.h>

struct boot_info_t {
  core_id_t                 core_id;
  size_t                    cap_count;
  map_ptr<root_boot_info_t> root_boot_info;
  map_ptr<task_t>           root_task;
  map_ptr<page_table_t>     root_page_table;
  map_ptr<cap_space_t>      cap_spaces[CONFIG_ROOT_TASK_CAP_SPACES];
  map_ptr<page_table_t>     payload_page_tables[NUM_INTER_PAGE_TABLE + 1];
  map_ptr<page_table_t>     stack_page_tables[NUM_INTER_PAGE_TABLE + 1];
  map_ptr<page_table_t>     cap_space_page_tables[NUM_INTER_PAGE_TABLE + 1];

  // ^^^ Generic ^^^ / vvv Arch Specific vvv

  map_ptr<char> dtb;
};

void                 init_boot_info(core_id_t core_id, map_ptr<char> dtb);
map_ptr<boot_info_t> get_boot_info();
size_t               get_root_boot_info_size(map_ptr<root_boot_info_t> root_boot_info);

#endif // ARCH_RV64_KERNEL_BOOT_INFO_H_
