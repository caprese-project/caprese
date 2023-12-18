#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>

#include <kernel/align.h>
#include <kernel/attribute.h>
#include <kernel/cap_space.h>
#include <kernel/cls.h>
#include <kernel/core_id.h>
#include <kernel/log.h>
#include <kernel/page.h>
#include <kernel/setup.h>
#include <kernel/start.h>
#include <kernel/task.h>
#include <kernel/trap.h>

extern "C" {
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

  map_ptr<boot_info_t> boot_info = get_boot_info();

  get_cls()->current_task = boot_info->root_task;

  init_task(boot_info->root_task, boot_info->cap_spaces[0], boot_info->root_page_table, boot_info->cap_space_page_tables);

  std::for_each(std::begin(boot_info->cap_spaces) + 1, std::end(boot_info->cap_spaces), [&boot_info](auto&& cap_space) {
    if (!insert_cap_space(boot_info->root_task, cap_space)) [[unlikely]] {
      panic("Failed to insert cap space.");
    }
  });

  constexpr int       flags = TASK_CAP_KILLABLE | TASK_CAP_SWITCHABLE | TASK_CAP_SUSPENDABLE | TASK_CAP_RESUMABLE | TASK_CAP_REGISTER_GETTABLE | TASK_CAP_REGISTER_SETTABLE | TASK_CAP_KILL_NOTIFIABLE;
  map_ptr<cap_slot_t> root_task_cap_slot = insert_cap(boot_info->root_task, make_task_cap(flags, boot_info->root_task));
  if (root_task_cap_slot == nullptr) [[unlikely]] {
    panic("Failed to insert the root task capability.");
  }
  boot_info->root_boot_info->root_task_cap = get_cap_slot_index(root_task_cap_slot);

  std::for_each(std::begin(boot_info->cap_spaces), std::end(boot_info->cap_spaces), [&boot_info](auto&& cap_space) {
    map_ptr<cap_slot_t> slot = insert_cap(boot_info->root_task, make_cap_space_cap(cap_space, true));
    if (slot == nullptr) [[unlikely]] {
      panic("Failed to insert the cap space capability.");
    }
  });

  map_ptr<cap_slot_t> root_page_table_cap_slot = insert_cap(boot_info->root_task, make_page_table_cap(boot_info->root_page_table, true, MAX_PAGE_TABLE_LEVEL, 0, 0_map));
  if (root_page_table_cap_slot == nullptr) [[unlikely]] {
    panic("Failed to insert the root page table capability.");
  }
  boot_info->root_boot_info->root_page_table_cap = get_cap_slot_index(root_page_table_cap_slot);

  for (size_t level = 0; level < std::size(boot_info->cap_space_page_tables); ++level) {
    map_ptr<page_table_t> page_table = boot_info->cap_space_page_tables[level];

    map_ptr<page_table_t> parent_table;
    if (level + 1 == MAX_PAGE_TABLE_LEVEL) [[unlikely]] {
      parent_table = boot_info->root_page_table;
    } else {
      parent_table = boot_info->cap_space_page_tables[level + 1];
    }

    map_ptr<cap_slot_t> slot = insert_cap(boot_info->root_task, make_page_table_cap(page_table, true, level, CONFIG_CAPABILITY_SPACE_BASE, parent_table));
    if (slot == nullptr) [[unlikely]] {
      panic("Failed to insert the cap space page table capability.");
    }

    boot_info->root_boot_info->cap_space_page_table_caps[level] = get_cap_slot_index(slot);
  }

  logi(tag, "Setting up the root task... done");
}

__init_code void setup_cap_space() {
  logi(tag, "Setting up capability space...");

  map_ptr<boot_info_t> boot_info = get_boot_info();

  boot_info->root_boot_info->num_mem_caps    = 0;
  boot_info->root_boot_info->mem_caps_offset = 0;
  setup_memory_capabilities(boot_info);

  logi(tag, "Setting up capability space... done");
}

template<size_t N, size_t M>
__init_code void map_root_task(virt_ptr<void> va_base, const char* begin, const char* end, map_ptr<page_table_t> (&page_tables)[N], page_table_cap_t (&dst)[M], pte_flags_t flags) {
  map_ptr<boot_info_t> boot_info = get_boot_info();

  std::for_each(std::begin(page_tables), std::end(page_tables), [](auto&& page_table) { memset(page_table.get(), 0, sizeof(page_table_t)); });

  map_ptr<page_table_t> page_table = boot_info->root_page_table;
  map_ptr<pte_t>        pte        = 0_map;

  for (size_t level = MAX_PAGE_TABLE_LEVEL; level > KILO_PAGE_TABLE_LEVEL; --level) {
    map_ptr<page_table_t> next_page_table = page_tables[level - 1];

    pte = page_table->walk(va_base, level);
    assert(pte->is_disabled());
    pte->set_next_page(next_page_table.as<void>());
    pte->enable();

    uintptr_t           virt_addr_base      = round_down(va_base.raw(), get_page_size(level));
    map_ptr<cap_slot_t> page_table_cap_slot = insert_cap(boot_info->root_task, make_page_table_cap(next_page_table, true, level - 1, virt_addr_base, page_table));
    if (page_table_cap_slot == nullptr) [[unlikely]] {
      panic("Failed to insert the page table capability.");
    }
    dst[level - 1] = get_cap_slot_index(page_table_cap_slot);
    page_table     = next_page_table;
  }

  size_t size = end - begin;
  for (uintptr_t va_offset = 0; va_offset < size; va_offset += PAGE_SIZE) {
    map_ptr<void> page = make_map_ptr(begin + va_offset);

    pte = page_table->walk(va_base + va_offset, KILO_PAGE_TABLE_LEVEL);
    assert(pte->is_disabled());
    pte->set_flags(flags);
    pte->set_next_page(page);
    pte->enable();

    map_ptr<cap_slot_t> virt_page_cap_slot = insert_cap(boot_info->root_task, make_virt_page_cap(flags.readable, flags.writable, flags.executable, KILO_PAGE_TABLE_LEVEL, page.as_phys().raw()));
    if (virt_page_cap_slot == nullptr) [[unlikely]] {
      panic("Failed to insert the virtual page capability.");
    }

    logd(tag, "Mapped page %p -> %p (4k)", va_base.raw() + va_offset, page.as_phys());
  }
}

__init_code void load_root_task_payload() {
  constexpr virt_ptr<void> va_base = make_virt_ptr(CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS);
  constexpr pte_flags_t    flags   = { .readable = 1, .writable = 1, .executable = 1, .user = 1, .global = 0 };

  map_ptr<boot_info_t> boot_info = get_boot_info();
  map_root_task(va_base, _payload_start, _payload_end, boot_info->payload_page_tables, boot_info->root_boot_info->page_table_caps, flags);
}

__init_code void setup_root_task_stack() {
  size_t                size    = static_cast<size_t>(_root_task_stack_end - _root_task_stack_start);
  virt_ptr<void>        va_base = make_virt_ptr(CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE - size);
  constexpr pte_flags_t flags   = { .readable = 1, .writable = 1, .executable = 0, .user = 1, .global = 0 };

  map_ptr<boot_info_t> boot_info = get_boot_info();
  map_root_task(va_base, _root_task_stack_start, _root_task_stack_end, boot_info->stack_page_tables, boot_info->root_boot_info->stack_page_table_caps, flags);
}

__init_code void setup_root_task_payload() {
  logi(tag, "Setting up the root task payload...");

  load_root_task_payload();
  setup_root_task_stack();

  map_ptr<boot_info_t> boot_info = get_boot_info();

  setup_arch_root_boot_info(boot_info);

  void*     stack     = bake_stack(make_map_ptr(_root_task_stack_end), boot_info->root_boot_info.as<void>(), get_root_boot_info_size(boot_info->root_boot_info));
  uintptr_t sp_offset = _root_task_stack_end - reinterpret_cast<char*>(stack);
  uintptr_t sp        = CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE - sp_offset;

  map_ptr<task_t> root_task = boot_info->root_task;

  set_register(make_map_ptr(&root_task->frame), REG_PROGRAM_COUNTER, CONFIG_ROOT_TASK_PAYLOAD_BASE_ADDRESS);
  set_register(make_map_ptr(&root_task->frame), REG_STACK_POINTER, sp);
  set_register(make_map_ptr(&root_task->frame), REG_ARG_0, sp);

  logi(tag, "Setting up the root task payload... done");
}

__init_code void setup_idle_task() {
  logi(tag, "Setting up the idle task...");

  task_t* task         = reinterpret_cast<task_t*>(get_cls()->idle_task_region);
  get_cls()->idle_task = make_map_ptr(task);

  init_idle_task(get_cls()->idle_task, make_map_ptr(get_cls()->idle_task_root_page_table));
}

__init_code void setup() {
  set_core_id(get_boot_info()->core_id);
  setup_early_trap();
  setup_root_task();
  setup_cap_space();
  setup_root_task_payload();
  setup_idle_task();
}

__init_code [[noreturn]] void start() {
  setup();

  lognl();

  for (const char* const line : logo) {
    logi(tag, line);
  }

  lognl();

  logi(tag, "Starting the root task...\n");

  load_context(make_map_ptr(&get_boot_info()->root_task->context));
}
