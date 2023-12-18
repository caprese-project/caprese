#include <algorithm>

#include <kernel/attribute.h>
#include <kernel/boot_info.h>

extern "C" {
  extern root_boot_info_t _root_boot_info;
}

namespace {
  __init_data boot_info_t boot_info;

  task_t       root_task;
  cap_space_t  root_task_cap_spaces[CONFIG_ROOT_TASK_CAP_SPACES];
  page_table_t root_task_root_page_table;
  page_table_t root_task_payload_page_tables[NUM_INTER_PAGE_TABLE + 1];
  page_table_t root_task_stack_page_tables[NUM_INTER_PAGE_TABLE + 1];
  page_table_t root_task_cap_space_page_tables[NUM_INTER_PAGE_TABLE + 1];

  static_assert(CONFIG_ROOT_TASK_CAP_SPACES < NUM_PAGE_TABLE_ENTRY);
} // namespace

void init_boot_info(core_id_t core_id, map_ptr<char> dtb) {
  boot_info.core_id         = core_id;
  boot_info.cap_count       = 0;
  boot_info.root_boot_info  = make_map_ptr(&_root_boot_info);
  boot_info.root_task       = make_map_ptr(&root_task);
  boot_info.root_page_table = make_map_ptr(&root_task_root_page_table);
  boot_info.dtb             = dtb;
  std::transform(std::begin(root_task_cap_spaces), std::end(root_task_cap_spaces), boot_info.cap_spaces, [](auto&& cap_space) { return make_map_ptr(&cap_space); });
  std::transform(std::begin(root_task_payload_page_tables), std::end(root_task_payload_page_tables), boot_info.payload_page_tables, [](auto&& page_table) { return make_map_ptr(&page_table); });
  std::transform(std::begin(root_task_stack_page_tables), std::end(root_task_stack_page_tables), boot_info.stack_page_tables, [](auto&& page_table) { return make_map_ptr(&page_table); });
  std::transform(std::begin(root_task_cap_space_page_tables), std::end(root_task_cap_space_page_tables), boot_info.cap_space_page_tables, [](auto&& page_table) { return make_map_ptr(&page_table); });
}

map_ptr<boot_info_t> get_boot_info() {
  return make_map_ptr(&boot_info);
}

size_t get_root_boot_info_size(map_ptr<root_boot_info_t> root_boot_info) {
  return sizeof(root_boot_info_t) + sizeof(cap_t) * (root_boot_info->num_mem_caps + root_boot_info->arch_info.num_dtb_vp_caps);
}
