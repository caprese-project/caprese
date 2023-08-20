#include <init/early_memory.h>
#include <init/main.h>
#include <lib/debug.h>
#include <lib/elf.h>
#include <lib/syscall.h>

namespace init {
  void main(boot_t* boot) {
    if (!init_early_memory(boot)) {
      printd("Failed to initialize early memories.\n");
      return;
    }

    for (size_t i = 0; i < NUM_INIT_TASKS; ++i) {
      auto& task = boot->tasks[i];

      size_t num_pages = elf_needed_pages(task.start, task.end - task.start);
      if (num_pages == 0) [[unlikely]] {
        printd("Failed to load elf.\n");
        return;
      }

      auto [task_cap, task_cap_error] = sys_task_create();
      if (task_cap_error) [[unlikely]] {
        printd("Failed to create task.\n");
        return;
      }

      cap_handle_t* mem_caps = static_cast<cap_handle_t*>(early_alloc(sizeof(cap_handle_t) * num_pages));
      if (mem_caps == nullptr) [[unlikely]] {
        printd("Out of memory.\n");
        return;
      }
      for (size_t i = 0; i < num_pages; ++i) {
        mem_caps[i] = fetch_mem_cap();
        if (get_cap_type(mem_caps[i]) == CAP_TYPE_NULL) [[unlikely]] {
          printd("Out of memory.\n");
          return;
        }
      }

      if (!load_elf(task_cap, mem_caps, task.start, task.end - task.start)) [[unlikely]] {
        printd("Failed to load elf.\n");
        return;
      }

      task.task_cap = task_cap;
    }

    early_free_all();

    for (size_t i = 0; i < NUM_INIT_TASKS; ++i) {
      boot->tasks[i].run(boot, boot->tasks[i].task_cap);
    }

    while (true) {
      // msg_t msg = sys_task_ipc_receive();
      // (void)msg;
    }
  }
} // namespace init
