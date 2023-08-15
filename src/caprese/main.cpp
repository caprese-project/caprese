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

#include <bit>
#include <cstdio>

#include <caprese/capability/builtin.h>
#include <caprese/main.h>
#include <caprese/memory/cls.h>
#include <caprese/memory/heap.h>
#include <caprese/memory/kernel_space.h>
#include <caprese/task/init.h>
#include <caprese/task/task.h>
#include <caprese/util/panic.h>

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
    [[noreturn]] void kernel_task_entry(const arch::boot_info_t* boot_info) {
      printf("Creating init task...\n");
      task::task_t* init_task = task::create_task();
      if (init_task == nullptr) [[unlikely]] {
        panic("Failed to create init task.");
      }
      printf("Init task creation completed. tid: 0x%x\n", std::bit_cast<uint32_t>(init_task->tid));
      printf("Loading the payload for the init task...\n");
      if (!task::load_init_task_payload(init_task, boot_info)) [[unlikely]] {
        panic("Failed to load init task payload.");
      }
      printf("Ready to execute the init task.\n");

      printf(LOGO_TEXT);

      printf("Switching to init task...\n\n");
      task::switch_to(init_task);

      while (true) { }
    }
  } // namespace

  [[noreturn]] void main(const arch::boot_info_t* boot_info) {
    printf("Initializing heap...\n");
    if (!memory::init_heap(boot_info)) [[unlikely]] {
      panic("Failed to initialize heap.");
    }
    printf("Heap initialization completed.\n\n");

    printf("Initializing core local storage...\n");
    if (!memory::init_cls_space(boot_info)) [[unlikely]] {
      panic("Failed to initialize core local storage.");
    }
    printf("Core local storage initialized.\n\n");

    printf("Initializing kernel virtual address space...\n");
    if (!memory::init_kernel_space()) [[unlikely]] {
      panic("Failed to initialize kernel virtual address space");
    }
    printf("Kernel virtual address space initialized.\n\n");

    printf("Initializing task space...\n");
    if (!task::init_task_space()) [[unlikely]] {
      panic("Failed to initialize task space.");
    }
    printf("Task space initialization completed.\n");
    printf("Creating kernel task...\n");
    task::task_t* kernel_task = task::create_kernel_task(kernel_task_entry, boot_info);
    if (kernel_task == nullptr) [[unlikely]] {
      panic("Failed to create kernel task.");
    }
    printf("Kernel task creation completed.\n\n");

    printf("Initializing capability class space...\n");
    if (!capability::init_capability_class_space()) [[unlikely]] {
      panic("Failed to initialize capability class space.");
    }
    printf("Capability class space initialization completed.\n");
    printf("Initializing capability space...\n");
    if (!capability::init_capability_space()) [[unlikely]] {
      panic("Failed to initialize capability space.");
    }
    printf("Capability space initialization completed.\n");
    printf("Creating built-in capability classes...\n");
    if (!capability::create_builtin_capability_classes()) [[unlikely]] {
      panic("Failed to create built-in capability classes.");
    }
    printf("Built-in capability classes creation completed.\n");
    printf("Creating built-in capabilities...\n");
    if (!capability::create_builtin_capabilities(kernel_task, boot_info)) [[unlikely]] {
      panic("Failed to create built-in capabilities.");
    }
    printf("Built-in capabilities creation completed.\n\n");

    printf("Switching to kernel task...\n\n");
    task::switch_to_kernel_task(kernel_task);

    panic("UNREACHABLE");
  }
} // namespace caprese
