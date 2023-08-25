#include <cstdlib>

#include <caprese/arch/builtin_capability.h>
#include <caprese/arch/memory.h>
#include <caprese/capability/bic/memory.h>
#include <caprese/capability/bic/task.h>
#include <caprese/capability/builtin.h>
#include <caprese/memory/heap.h>
#include <caprese/task/cap.h>

namespace caprese::capability {
  bool create_builtin_capability_classes() {
    if (bic::memory::create_class() == nullptr) [[unlikely]] {
      return false;
    }

    if (bic::task::create_class() == nullptr) [[unlikely]] {
      return false;
    }

    return arch::create_builtin_capability_classes();
  }

  bool create_builtin_capabilities(task::task_t* kernel_task, const arch::boot_info_t* boot_info) {
    while (memory::num_remaining_pages() > CONFIG_KERNEL_RESERVED_PAGES) {
      using namespace bic::memory;

      memory::mapped_address_t page = memory::mapped_address_t::from(aligned_alloc(arch::PAGE_SIZE, arch::PAGE_SIZE));
      if (page.is_null()) [[unlikely]] {
        return false;
      }

      capability_t* cap = create(page.physical_address(), constant::READABLE | constant::WRITABLE | constant::EXECUTABLE);
      if (cap == nullptr) [[unlikely]] {
        return false;
      }

      if (task::insert_capability(kernel_task, cap) == 0) [[unlikely]] {
        return false;
      }
    }

    return arch::create_builtin_capabilities(kernel_task, boot_info);
  }
} // namespace caprese::capability
