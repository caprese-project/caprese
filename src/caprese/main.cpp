/**
 * @file main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement platform-independent kernel main.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#include <cstdio>

#include <caprese/capability/init.h>
#include <caprese/main.h>
#include <caprese/memory/heap.h>
#include <caprese/task/init.h>
#include <caprese/task/task.h>

constexpr auto LOGO_TEXT = R"(
  ____
 / ___|  __ _  _ __   _ __   ___  ___   ___
| |     / _` || '_ \ | '__| / _ \/ __| / _ \
| |___ | (_| || |_) || |   |  __/\__ \|  __/
 \____| \__,_|| .__/ |_|    \___||___/ \___|
              |_|

)";

namespace caprese {
  namespace {
    [[noreturn]] void kernel_task_entry(arch::boot_info_t* boot_info) {
      printf("Creating init task...\n");
      task::task_t* init_task = task::create_task();
      printf("Init task creation completed.\n");
      printf("Loading the payload for the init task...\n");
      task::load_init_task_payload(init_task, boot_info);
      printf("Ready to execute the init task.\n\n");

      printf(LOGO_TEXT);

      printf("Switching to init task...\n\n");
      task::switch_to(init_task);

      while (true) { }
    }
  } // namespace

  [[noreturn]] void main(arch::boot_info_t* boot_info) {
    printf("Initializing heap...\n");
    memory::init_heap_space(boot_info);
    printf("Heap initialization completed.\n\n");

    printf("Initializing task space...\n");
    task::init_task_space();
    printf("Task space initialization completed.\n");
    printf("Creating kernel task...\n");
    task::task_t* kernel_task = task::create_kernel_task(kernel_task_entry, boot_info);
    printf("Kernel task creation completed.\n\n");

    printf("Initializing capability space...\n");
    capability::init_capability_space();
    printf("Capability space initialization completed.\n\n");
    printf("Creating initial capabilities...\n");
    capability::create_init_capabilities(kernel_task, boot_info);
    printf("Initial capabilities creation completed.\n\n");

    printf("Switching to kernel task...\n\n");
    task::switch_to(kernel_task);

    while (true) {}
  }
} // namespace caprese
