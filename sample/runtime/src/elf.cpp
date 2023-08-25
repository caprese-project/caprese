#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstring>

#include <caprese/arch/task.h>
#include <caprese/capability/bic/task.h>
#include <caprese/util/align.h>

#include <runtime/elf.h>
#include <runtime/global.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>
#include <runtime/task.h>

namespace {
  bool is_valid_elf_header(const elf_header_t* elf_header) {
    if (elf_header == nullptr) [[unlikely]] {
      return false;
    }

    if (elf_header->magic[0] != ELF_MAGIC0) [[unlikely]] {
      return false;
    }
    if (elf_header->magic[1] != ELF_MAGIC1) [[unlikely]] {
      return false;
    }
    if (elf_header->magic[2] != ELF_MAGIC2) [[unlikely]] {
      return false;
    }
    if (elf_header->magic[3] != ELF_MAGIC3) [[unlikely]] {
      return false;
    }

    if (elf_header->xlen != ELF_CURRENT_XLEN) [[unlikely]] {
      return false;
    }

    if constexpr (std::endian::native == std::endian::little) {
      if (elf_header->endian != ELF_LITTLE_ENDIAN) [[unlikely]] {
        return false;
      }
    } else {
      if (elf_header->endian != ELF_BIG_ENDIAN) [[unlikely]] {
        return false;
      }
    }

    if (elf_header->elf_header_version != ELF_CURRENT_VERSION) [[unlikely]] {
      return false;
    }

    if (elf_header->machine != ELF_CURRENT_MACHINE) [[unlikely]] {
      return false;
    }

    return true;
  }
} // namespace

extern "C" {
  bool load_elf(task_handle_t task, const void* src) {
    const elf_header_t* elf = static_cast<const elf_header_t*>(src);

    if (task_state(task) != TASK_STATE_CREATING) [[unlikely]] {
      return false;
    }

    if (!is_valid_elf_header(elf)) [[unlikely]] {
      return false;
    }

    if (elf->type != ELF_TYPE_EXECUTABLE) [[unlikely]] {
      return false;
    }

    uintptr_t heap_start = 0;

    const elf_program_header_t* program_headers = reinterpret_cast<const elf_program_header_t*>(static_cast<const char*>(src) + elf->program_header_position);
    for (size_t i = 0; i < elf->num_program_header_entries; ++i) {
      if (program_headers[i].segment_type != ELF_PH_SEGMENT_LOAD) {
        continue;
      }

      if (program_headers[i].file_size > program_headers[i].memory_size) [[unlikely]] {
        return false;
      }

      unsigned flags = 0;
      if (program_headers[i].flags & ELF_PH_FLAG_READABLE) {
        flags |= MEMORY_FLAG_READABLE;
      }
      if (program_headers[i].flags & ELF_PH_FLAG_WRITABLE) {
        flags |= MEMORY_FLAG_WRITABLE;
      }
      if (program_headers[i].flags & ELF_PH_FLAG_EXECUTABLE) {
        flags |= MEMORY_FLAG_EXECUTABLE;
      }

      const auto map_header = [&](uintptr_t offset, const void* data, size_t size) {
        memory_handle_t handle = fetch_memory_handle(MEMORY_FLAG_WRITABLE | flags);
        if (handle == 0) [[unlikely]] {
          return false;
        }

        uintptr_t va = get_virtual_address(handle);
        if (va == 0) [[unlikely]] {
          return false;
        }

        char* page = reinterpret_cast<char*>(va);
        memcpy(page, data, size);
        memset(page + size, 0, __page_size - size);

        if (!unmap_memory_page(handle)) [[unlikely]] {
          return false;
        }
        if (!map_memory_page_to_task(task, handle, caprese::round_down(program_headers[i].virtual_address, __page_size) + offset, flags)) [[unlikely]] {
          return false;
        }
        if (sys_cap_move(handle, task).error) [[unlikely]] {
          return false;
        }

        return true;
      };

      for (size_t offset = 0; offset < program_headers[i].file_size; offset += __page_size) {
        if (!map_header(offset, static_cast<const char*>(src) + program_headers[i].offset + offset, std::min(program_headers[i].file_size - offset, __page_size))) [[unlikely]] {
          return false;
        }
      }
      for (size_t offset = caprese::round_up(program_headers[i].file_size, __page_size); offset < program_headers[i].memory_size; offset += __page_size) {
        if (!map_header(offset, nullptr, 0)) [[unlikely]] {
          return false;
        }
      }

      heap_start = std::max(heap_start, program_headers[i].virtual_address + program_headers[i].memory_size);
    }

    memory_handle_t stack_handle = fetch_memory_handle(MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE);
    if (stack_handle == 0) [[unlikely]] {
      return false;
    }
    uintptr_t stack_va = get_virtual_address(stack_handle);
    if (stack_va == 0) [[unlikely]] {
      return false;
    }

    memset(reinterpret_cast<void*>(stack_va), 0, __page_size);

    if (!unmap_memory_page(stack_handle)) [[unlikely]] {
      return false;
    }

    uintptr_t stack_page = __user_space_end - __page_size;
    if (!map_memory_page_to_task(task, stack_handle, stack_page, MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE)) [[unlikely]] {
      return false;
    }
    if (sys_cap_move(stack_handle, task).error) [[unlikely]] {
      return false;
    }

    if (!set_register(task, caprese::arch::ABI_STACK_POINTER, __user_space_end)) [[unlikely]] {
      return false;
    }
    if (!set_register(task, caprese::arch::ABI_PROGRAM_COUNTER, elf->entry_position)) [[unlikely]] {
      return false;
    }

    sysret_t sysret;

    sysret = sys_cap_copy(__init_task_handle, caprese::capability::bic::task::permission::ALL);
    if (sysret.error) [[unlikely]] {
      return false;
    }
    sysret = sys_cap_move(sysret.result, task);
    if (sysret.error) [[unlikely]] {
      return false;
    }
    if (!set_register(task, caprese::arch::ABI_ARGUMENTS[0], sysret.result)) [[unlikely]] {
      return false;
    }

    sysret = sys_cap_copy(task, caprese::capability::bic::task::permission::ALL);
    if (sysret.error) [[unlikely]] {
      return false;
    }
    sysret = sys_cap_move(sysret.result, task);
    if (sysret.error) [[unlikely]] {
      return false;
    }
    if (!set_register(task, caprese::arch::ABI_ARGUMENTS[1], sysret.result)) [[unlikely]] {
      return false;
    }

    if (__apm_task_handle != 0) {
      sysret = sys_cap_copy(__apm_task_handle, caprese::capability::bic::task::permission::ALL);
      if (sysret.error) [[unlikely]] {
        return false;
      }
      sysret = sys_cap_move(sysret.result, task);
      if (sysret.error) [[unlikely]] {
        return false;
      }
      if (!set_register(task, caprese::arch::ABI_ARGUMENTS[2], sysret.result)) [[unlikely]] {
        return false;
      }
    } else {
      if (!set_register(task, caprese::arch::ABI_ARGUMENTS[2], 0)) [[unlikely]] {
        return false;
      }
    }

    if (!set_register(task, caprese::arch::ABI_ARGUMENTS[3], caprese::round_up(heap_start, __page_size))) [[unlikely]] {
      return false;
    }

    return true;
  }
}
