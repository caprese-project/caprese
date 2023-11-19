#include <cassert>
#include <cstring>
#include <iterator>

#include <kernel/attribute.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/core_id.h>
#include <kernel/page.h>
#include <kernel/setup.h>
#include <kernel/start.h>
#include <kernel/task.h>
#include <kernel/trap.h>
#include <libcaprese/root_boot_info.h>
#include <log/log.h>

extern "C" {
  extern const char _kernel_start[];
  extern const char _kernel_end[];
  extern const char _payload_start[];
  extern const char _payload_end[];
  extern char       _root_task_stack_start[];
  extern char       _root_task_stack_end[];
}

namespace {
  constexpr const char* tag = "kernel/start";

  // clang-format off

  constexpr const char* const logo[] = {
    R"(  ____                                      )",
    R"( / ___|  __ _  _ __   _ __   ___  ___   ___ )",
    R"(| |     / _` || '_ \ | '__| / _ \/ __| / _ \)",
    R"(| |___ | (_| || |_) || |   |  __/\__ \|  __/)",
    R"( \____| \__,_|| .__/ |_|    \___||___/ \___|)",
    R"(              |_|                           )",
  };

  // clang-format on

  task_t       root_task;
  cap_space_t  root_task_first_cap_space;
  page_table_t root_task_root_page_table;
  page_table_t root_task_page_tables[NUM_PAGE_TABLE_LEVEL - 1];
  page_table_t root_task_cap_space_page_tables[MAX_PAGE_TABLE_LEVEL - MEGA_PAGE_TABLE_LEVEL + 1];

  __init_data root_boot_info_t alignas(PAGE_SIZE) root_boot_info;
} // namespace

__init_code [[noreturn]] void early_trap_handler() {
  logf("KERNEL CRITICAL ERROR", "Reached the early trap handler.");
  logf("KERNEL CRITICAL ERROR", "This error indicates a bug or improper behavior in the kernel.");
  logf("KERNEL CRITICAL ERROR", "Detailed information and register dump will follow...");
  panic("Early trap!");
}

__init_code void setup_early_trap() {
  logi(tag, "Setting up early trap handler...");
  set_trap_handler(early_trap_handler);
  logi(tag, "Setting up early trap handler... done");
}

__init_code void setup_root_task() {
  logi(tag, "Setting up the root task...");

  get_cls()->current_task = &root_task;

  page_table_t* tables[std::size(root_task_cap_space_page_tables)];
  for (size_t i = 0; i < std::size(root_task_cap_space_page_tables); ++i) {
    tables[i] = &root_task_cap_space_page_tables[i];
  }

  init_task(&root_task, &root_task_first_cap_space, &root_task_root_page_table, tables);

  constexpr int flags = TASK_CAP_KILLABLE | TASK_CAP_SWITCHABLE | TASK_CAP_SUSPENDABLE | TASK_CAP_RESUMABLE | TASK_CAP_REGISTER_GETTABLE | TASK_CAP_REGISTER_SETTABLE | TASK_CAP_KILL_NOTIFIABLE;
  cap_slot_t*   root_task_cap_slot = insert_cap(&root_task, make_task_cap(flags, &root_task));
  if (root_task_cap_slot == nullptr) [[unlikely]] {
    panic("Failed to insert the root task capability.");
  }
  root_boot_info.root_task_cap = get_cap_slot_index(root_task_cap_slot);

  cap_slot_t* root_task_first_cap_space_cap_slot = insert_cap(&root_task, make_cap_space_cap(&root_task_first_cap_space));
  if (root_task_first_cap_space_cap_slot == nullptr) [[unlikely]] {
    panic("Failed to insert the first cap space capability.");
  }
  root_boot_info.first_cap_space_cap = get_cap_slot_index(root_task_first_cap_space_cap_slot);

  cap_slot_t* root_task_root_page_table_cap_slot = insert_cap(&root_task, make_page_table_cap(MAX_PAGE_TABLE_LEVEL, &root_task_root_page_table));
  if (root_task_root_page_table_cap_slot == nullptr) [[unlikely]] {
    panic("Failed to insert the root page table capability.");
  }
  root_boot_info.root_page_table_cap = get_cap_slot_index(root_task_root_page_table_cap_slot);

  cap_slot_t* root_task_cap_space_page_table_cap_slots[std::size(root_task_cap_space_page_tables)];
  for (size_t i = 0; i < std::size(root_task_cap_space_page_tables); ++i) {
    root_task_cap_space_page_table_cap_slots[i] = insert_cap(&root_task, make_page_table_cap(i, &root_task_cap_space_page_tables[i]));
    if (root_task_cap_space_page_table_cap_slots[i] == nullptr) [[unlikely]] {
      panic("Failed to insert the cap space page table capability.");
    }
    root_boot_info.cap_space_page_table_caps[i] = get_cap_slot_index(root_task_cap_space_page_table_cap_slots[i]);
  }

  logi(tag, "Setting up the root task... done");
}

__init_code void setup_cap_space(boot_info_t* boot_info) {
  logi(tag, "Setting up capability space...");

  root_boot_info.num_mem_caps = 0;
  setup_memory_capabilities(&root_task, boot_info, &root_boot_info);

  logi(tag, "Setting up capability space... done");
}

__init_code void setup_root_task_payload() {
  logi(tag, "Setting up the root task payload...");

  constexpr virt_addr_t payload_base_va = virt_addr_t::from(CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS);

  memset(root_task_page_tables, 0, sizeof(root_task_page_tables));

  page_table_t* page_table = &root_task_root_page_table;
  pte_t*        pte        = nullptr;
  for (size_t level = MAX_PAGE_TABLE_LEVEL; level > KILO_PAGE_TABLE_LEVEL; --level) {
    pte = page_table->walk(payload_base_va, level);
    assert(pte->is_disabled());
    pte->set_next_page(map_addr_t::from(&root_task_page_tables[level - 1]));
    pte->enable();
    page_table = pte->get_next_page().as<page_table_t*>();
  }

  for (uintptr_t va_offset = 0; va_offset < static_cast<size_t>(_payload_end - _payload_start); va_offset += PAGE_SIZE) {
    pte = page_table->walk(payload_base_va + va_offset, KILO_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags({ .readable = 1, .writable = 1, .executable = 1, .user = 1, .global = 0 });
    pte->set_next_page(map_addr_t::from(_payload_start + va_offset));
    pte->enable();
  }

  void*     stack     = bake_stack(_root_task_stack_end, &root_boot_info, sizeof(root_boot_info_t) + sizeof(mem_cap_t) * root_boot_info.num_mem_caps);
  uintptr_t sp_offset = reinterpret_cast<char*>(stack) - _payload_start;
  uintptr_t sp        = CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS + sp_offset;

  set_register(&root_task.frame, REG_PROGRAM_COUNTER, CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS);
  set_register(&root_task.frame, REG_STACK_POINTER, sp);
  set_register(&root_task.frame, REG_ARG_0, sp);

  logi(tag, "Setting up the root task payload... done");
}

__init_code void setup(boot_info_t* boot_info) {
  set_core_id(boot_info->core_id);

  setup_early_trap();
  setup_root_task();
  setup_cap_space(boot_info);
  setup_root_task_payload();
}

__init_code [[noreturn]] void start(boot_info_t* boot_info) {
  setup(boot_info);

  lognl();

  for (const char* const line : logo) {
    logi(tag, line);
  }

  lognl();

  logi(tag, "Starting the init task...\n");

  load_context(&root_task.context);
}
