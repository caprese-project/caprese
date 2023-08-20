#include <algorithm>
#include <bit>
#include <cstring>

#include <caprese/util/align.h>

#include <lib/abi.h>
#include <lib/debug.h>
#include <lib/elf.h>
#include <lib/syscall.h>

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

#if defined(CONFIG_XLEN_32)
    if (elf_header->xlen != ELF_XLEN_32) [[unlikely]] {
      return false;
    }
#elif defined(CONFIG_XLEN_64)
    if (elf_header->xlen != ELF_XLEN_64) [[unlikely]] {
      return false;
    }
#endif

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

#if defined(CONFIG_ARCH_RISCV)
    if (elf_header->machine != ELF_MACHINE_RISCV) [[unlikely]] {
      return false;
    }
#endif

    return true;
  }
} // namespace

size_t elf_needed_pages(const char* data, size_t size) {
  const elf_header_t* elf_header = reinterpret_cast<const elf_header_t*>(data);
  if (!is_valid_elf_header(elf_header)) [[unlikely]] {
    return 0;
  }

  if (elf_header->type != ELF_TYPE_EXECUTABLE) [[unlikely]] {
    return 0;
  }

  size_t result = 0;

  auto [page_size, page_size_error] = sys_base_page_size();
  if (page_size_error) [[unlikely]] {
    return false;
  }

  printd("elf:\n");
  printd("  type:                       0x%x\n", elf_header->type);
  printd("  machine:                    0x%x\n", elf_header->machine);
  printd("  elf version:                0x%x\n", elf_header->elf_version);
  printd("  entry point:                0x%lx\n", elf_header->entry_position);
  printd("  program header position:    0x%lx\n", elf_header->program_header_position);
  printd("  section header position:    0x%lx\n", elf_header->section_header_position);
  printd("  flags:                      0x%x\n", elf_header->flags);
  printd("  header size:                0x%x\n", elf_header->header_size);
  printd("  program header entry size:  0x%x\n", elf_header->program_header_entry_size);
  printd("  num program header entries: 0x%x\n", elf_header->num_program_header_entries);
  printd("  section header entry size:  0x%x\n", elf_header->section_header_entry_size);
  printd("  num section header entries: 0x%x\n", elf_header->num_section_header_entries);
  printd("  index of section header:    0x%x\n", elf_header->idx_section_header);

  const elf_program_header_t* program_headers = reinterpret_cast<const elf_program_header_t*>(data + elf_header->program_header_position);
  for (size_t i = 0; i < elf_header->num_program_header_entries; ++i) {
    printd("  program header[%d]:\n", i);
    printd("    type:        ");
    switch (program_headers[i].segment_type) {
      case ELF_PH_SEGMENT_NULL:
        printd("null\n");
        break;
      case ELF_PH_SEGMENT_LOAD:
        printd("load\n");
        break;
      case ELF_PH_SEGMENT_DYNAMIC:
        printd("dynamic\n");
        break;
      case ELF_PH_SEGMENT_INTERPRET:
        printd("interpret\n");
        break;
      case ELF_PH_SEGMENT_NOTE:
        printd("note\n");
        break;
      default:
        printd("unknown(0x%x)\n", program_headers[i].segment_type);
        break;
    }
    printd("    flags:       (%c%c%c)\n",
           (program_headers[i].flags & ELF_PH_FLAG_READABLE) ? 'R' : ' ',
           (program_headers[i].flags & ELF_PH_FLAG_WRITABLE) ? 'W' : ' ',
           (program_headers[i].flags & ELF_PH_FLAG_EXECUTABLE) ? 'X' : ' ');
    printd("    offset:      0x%lx\n", program_headers[i].offset);
    printd("    virt addr:   0x%lx\n", program_headers[i].virtual_address);
    printd("    phys addr:   0x%lx\n", program_headers[i].physical_address);
    printd("    file size:   0x%lx\n", program_headers[i].file_size);
    printd("    memory size: 0x%lx\n", program_headers[i].memory_size);
    printd("    alignment:   0x%lx\n", program_headers[i].alignment);

    if (program_headers[i].segment_type != ELF_PH_SEGMENT_LOAD) {
      continue;
    }

    if (program_headers[i].file_size > program_headers[i].memory_size) [[unlikely]] {
      return 0;
    }

    if (program_headers[i].offset + program_headers[i].file_size > size) [[unlikely]] {
      return 0;
    }

    result += caprese::round_up(program_headers[i].memory_size, program_headers[i].alignment);
    result = caprese::round_up(result, page_size);
  }

  return result / page_size + 1; // sp
}

bool load_elf(cap_handle_t task_cap, cap_handle_t* mem_caps, const char* data, size_t size) {
  const elf_header_t* elf_header = reinterpret_cast<const elf_header_t*>(data);
  if (!is_valid_elf_header(elf_header)) [[unlikely]] {
    return false;
  }

  if (elf_header->type != ELF_TYPE_EXECUTABLE) [[unlikely]] {
    return false;
  }

  const auto fetch_memory = [&]() -> std::pair<void*, cap_handle_t> {
    if (get_cap_type(*mem_caps) != CAP_TYPE_MEMORY) [[unlikely]] {
      return { nullptr, 0 };
    }

    auto [cur_tid, cur_tid_error] = sys_task_tid();
    if (cur_tid_error) [[unlikely]] {
      return { nullptr, 0 };
    }

    auto [cap_tid, cap_tid_error] = sys_cap_get_field(*mem_caps, MEMORY_CAP_FIELD_TID);
    if (cap_tid_error) [[unlikely]] {
      return { nullptr, 0 };
    }

    if (cur_tid != cap_tid) [[unlikely]] {
      return { nullptr, 0 };
    }

    auto [va, va_error] = sys_cap_get_field(*mem_caps, MEMORY_CAP_FIELD_VIRTUAL_ADDRESS);
    if (va_error) [[unlikely]] {
      return { nullptr, 0 };
    }

    return { reinterpret_cast<void*>(va), *mem_caps++ };
  };

  auto [page_size, page_size_error] = sys_base_page_size();
  if (page_size_error) [[unlikely]] {
    return false;
  }

  uintptr_t stack_base = 0;

  const elf_program_header_t* program_headers = reinterpret_cast<const elf_program_header_t*>(data + elf_header->program_header_position);
  for (size_t i = 0; i < elf_header->num_program_header_entries; ++i) {
    if (program_headers[i].segment_type != ELF_PH_SEGMENT_LOAD) {
      continue;
    }

    if (program_headers[i].offset + program_headers[i].file_size > size) [[unlikely]] {
      return false;
    }

    uint8_t flags = 0;
    if (program_headers[i].flags & ELF_PH_FLAG_READABLE) {
      flags |= MEMORY_CAP_CONSTANT_READABLE;
    }
    if (program_headers[i].flags & ELF_PH_FLAG_WRITABLE) {
      flags |= MEMORY_CAP_CONSTANT_WRITABLE;
    }
    if (program_headers[i].flags & ELF_PH_FLAG_EXECUTABLE) {
      flags |= MEMORY_CAP_CONSTANT_EXECUTABLE;
    }

    for (size_t offset = 0; offset < program_headers[i].file_size; offset += page_size) {
      auto [page, cap] = fetch_memory();
      if (page == nullptr) [[unlikely]] {
        return false;
      }

      memset(page, 0, page_size);
      memcpy(page, data + program_headers[i].offset + offset, std::min(program_headers[i].file_size - offset, page_size));

      sysret_t result = sys_cap_call_method(cap, MEMORY_CAP_METHOD_UNMAP);
      if (result.error) [[unlikely]] {
        return false;
      }
      result = sys_cap_call_method(cap, MEMORY_CAP_METHOD_MAP, task_cap, program_headers[i].virtual_address + offset, flags);
      if (result.error) [[unlikely]] {
        return false;
      }
      result = sys_cap_move(cap, task_cap);
      if (result.error) [[unlikely]] {
        return false;
      }
    }

    for (size_t offset = caprese::round_up(program_headers[i].file_size, page_size); offset < program_headers[i].memory_size; offset += page_size) {
      auto [page, cap] = fetch_memory();
      if (page == nullptr) [[unlikely]] {
        return false;
      }

      memset(page, 0, page_size);

      sysret_t result = sys_cap_call_method(cap, MEMORY_CAP_METHOD_UNMAP);
      if (result.error) [[unlikely]] {
        return false;
      }
      result = sys_cap_call_method(cap, MEMORY_CAP_METHOD_MAP, task_cap, program_headers[i].virtual_address + offset, flags);
      if (result.error) [[unlikely]] {
        return false;
      }
      result = sys_cap_move(cap, task_cap);
      if (result.error) [[unlikely]] {
        return false;
      }
    }

    stack_base = std::max(stack_base, program_headers[i].virtual_address + program_headers[i].memory_size);
  }

  stack_base              = caprese::round_up(stack_base, page_size);
  uintptr_t stack_pointer = stack_base + page_size;

  auto [stack_page, stack_page_cap] = fetch_memory();
  if (stack_page == nullptr) [[unlikely]] {
    return false;
  }

  memset(stack_page, 0, page_size);

  sysret_t result = sys_cap_call_method(stack_page_cap, MEMORY_CAP_METHOD_UNMAP);
  if (result.error) [[unlikely]] {
    return false;
  }
  result = sys_cap_call_method(stack_page_cap, MEMORY_CAP_METHOD_MAP, task_cap, stack_base, MEMORY_CAP_CONSTANT_READABLE | MEMORY_CAP_CONSTANT_WRITABLE);
  if (result.error) [[unlikely]] {
    return false;
  }
  result = sys_cap_move(stack_page_cap, task_cap);
  if (result.error) [[unlikely]] {
    return false;
  }

  result = sys_cap_call_method(task_cap, TASK_CAP_METHOD_SET_REGISTER, ABI_STACK_POINTER, stack_pointer);
  if (result.error) [[unlikely]] {
    return false;
  }
  result = sys_cap_call_method(task_cap, TASK_CAP_METHOD_SET_REGISTER, ABI_PROGRAM_COUNTER, elf_header->entry_position);
  if (result.error) [[unlikely]] {
    return false;
  }

  return result.error == 0;
}
