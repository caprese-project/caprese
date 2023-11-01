#include <cassert>
#include <expected>

#include <kernel/attribute.h>
#include <kernel/core_id.h>
#include <kernel/page.h>
#include <kernel/start.h>
#include <kernel/trap.h>
#include <log/log.h>

extern "C" {
  extern const char _kernel_start[];
  extern const char _kernel_end[];
  extern const char _payload_start[];
  extern const char _payload_end[];
  extern const char _init_task_stack_start[];
  extern const char _init_task_stack_end[];
}

// clang-format off

namespace {
  constexpr const char* tag = "kernel/start";
} // namespace

constexpr const char* const LOGO_TEXT[] = {
  R"(  ____                                      )",
  R"( / ___|  __ _  _ __   _ __   ___  ___   ___ )",
  R"(| |     / _` || '_ \ | '__| / _ \/ __| / _ \)",
  R"(| |___ | (_| || |_) || |   |  __/\__ \|  __/)",
  R"( \____| \__,_|| .__/ |_|    \___||___/ \___|)",
  R"(              |_|                           )",
};

// clang-format on

__init_code [[noreturn]] void early_trap_handler() {
  logf("KERNEL CRITICAL ERROR", "Reached the early trap handler.");
  logf("KERNEL CRITICAL ERROR", "This error indicates a bug or improper behavior in the kernel.");
  logf("KERNEL CRITICAL ERROR", "Detailed information and register dump will follow...");
  panic("Early trap!");
}

__init_code void setup_early_trap() {
  logd("kernel/start", "Setting up early trap handler...");
  set_trap_handler(early_trap_handler);
  logd("kernel/start", "Setting up early trap handler... done");
}

__init_code void setup_page_table() {
  logd("kernel/start", "Setting up page table...");

  // constexpr virt_addr_t cap_space_base_va = virt_addr_t::from(CONFIG_CAPABILITY_SPACE_BASE);

  // page_table_t* page_table = map_addr_t::from(_init_task_root_page_table).as<page_table_t*>();
  // pte_t*        pte        = nullptr;
  // for (int level = MAX_PAGE_TABLE_LEVEL; level >= static_cast<int>(MEGA_PAGE_TABLE_LEVEL); --level) {
  //   pte = page_table->walk(cap_space_base_va, level);
  //   assert(pte->is_disabled());
  //   pte->set_next_page(map_addr_t::from(_init_task_cap_space_page_tables + (PAGE_SIZE * (MAX_PAGE_TABLE_LEVEL - level))));
  //   pte->set_flags({});
  //   pte->enable();
  //   page_table = pte->get_next_page().as<page_table_t*>();
  // }

  // constexpr virt_addr_t payload_base_va = virt_addr_t::from(CONFIG_INIT_TASK_PAYLOAD_BASE_ADDRESS);

  // page_table = map_addr_t::from(_init_task_root_page_table).as<page_table_t*>();
  // pte        = nullptr;
  // for (int level = MAX_PAGE_TABLE_LEVEL; level > 0; --level) {
  //   pte = page_table->walk(payload_base_va, level);
  //   assert(pte->is_disabled());
  //   pte->set_next_page(map_addr_t::from(_init_task_page_tables + (PAGE_SIZE * (MAX_PAGE_TABLE_LEVEL - level))));
  //   pte->set_flags({});
  //   pte->enable();
  //   page_table = pte->get_next_page().as<page_table_t*>();
  // }

  logd("kernel/start", "Setting up page table... done");
}

__init_code void setup(const boot_info_t* boot_info) {
  set_core_id(boot_info->core_id);

  setup_early_trap();
  setup_page_table();
}

__init_code [[noreturn]] void start(const boot_info_t* boot_info) {
  setup(boot_info);

  lognl();

  for (const char* const line : LOGO_TEXT) {
    logi(tag, line);
  }

  lognl();

  logi(tag, "Starting the init task...\n");

  panic("Kernel not implemented!");
}
