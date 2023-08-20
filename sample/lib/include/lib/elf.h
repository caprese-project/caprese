#ifndef LIB_ELF_H_
#define LIB_ELF_H_

#include <cstddef>
#include <cstdint>

#include <lib/capability.h>

using elf_half_t  = uint16_t;
using elf_word_t  = uint32_t;
using elf_sword_t = int32_t;
#if defined(CONFIG_XLEN_32)
using elf_off_t  = uint32_t;
using elf_addr_t = uint32_t;
#elif defined(CONFIG_XLEN_64)
using elf_off_t  = uint64_t;
using elf_addr_t = uint64_t;
#endif // CONFIG_XLEN

constexpr uint8_t ELF_MAGIC0 = '\x7f';
constexpr uint8_t ELF_MAGIC1 = 'E';
constexpr uint8_t ELF_MAGIC2 = 'L';
constexpr uint8_t ELF_MAGIC3 = 'F';

constexpr uint8_t ELF_XLEN_32 = 1;
constexpr uint8_t ELF_XLEN_64 = 2;

constexpr uint8_t ELF_LITTLE_ENDIAN = 1;
constexpr uint8_t ELF_BIG_ENDIAN    = 2;

constexpr uint8_t ELF_CURRENT_VERSION = 1;

constexpr elf_half_t ELF_TYPE_NONE        = 0;
constexpr elf_half_t ELF_TYPE_RELOCATABLE = 1;
constexpr elf_half_t ELF_TYPE_EXECUTABLE  = 2;
constexpr elf_half_t ELF_TYPE_SHARED      = 3;
constexpr elf_half_t ELF_TYPE_CORE        = 4;

constexpr elf_half_t ELF_MACHINE_RISCV = 0xf3;

struct elf_header_t {
  uint8_t    magic[4];
  uint8_t    xlen;
  uint8_t    endian;
  uint8_t    elf_header_version;
  uint8_t    os_abi;
  uint8_t    os_abi_version;
  uint8_t    pad[7];
  elf_half_t type;
  elf_half_t machine;
  elf_word_t elf_version;
  elf_addr_t entry_position;
  elf_off_t  program_header_position;
  elf_off_t  section_header_position;
  elf_word_t flags;
  elf_half_t header_size;
  elf_half_t program_header_entry_size;
  elf_half_t num_program_header_entries;
  elf_half_t section_header_entry_size;
  elf_half_t num_section_header_entries;
  elf_half_t idx_section_header;
};

constexpr elf_word_t ELF_PH_SEGMENT_NULL      = 0;
constexpr elf_word_t ELF_PH_SEGMENT_LOAD      = 1;
constexpr elf_word_t ELF_PH_SEGMENT_DYNAMIC   = 2;
constexpr elf_word_t ELF_PH_SEGMENT_INTERPRET = 3;
constexpr elf_word_t ELF_PH_SEGMENT_NOTE      = 4;

constexpr elf_word_t ELF_PH_FLAG_EXECUTABLE = 1 << 0;
constexpr elf_word_t ELF_PH_FLAG_WRITABLE   = 1 << 1;
constexpr elf_word_t ELF_PH_FLAG_READABLE   = 1 << 2;

struct elf_program_header_t {
  elf_word_t segment_type;
#if defined(CONFIG_XLEN_64)
  elf_word_t flags;
#endif // defined(CONFIG_XLEN_64)
  elf_off_t  offset;
  elf_addr_t virtual_address;
  elf_addr_t physical_address;
  elf_off_t  file_size;
  elf_off_t  memory_size;
#if defined(CONFIG_XLEN_32)
  elf_word_t flags;
#endif // defined(CONFIG_XLEN_32)
  elf_off_t alignment;
};

struct elf_section_header_t {
  elf_word_t name;
  elf_word_t type;
  elf_word_t flags;
  elf_addr_t address;
  elf_off_t  offset;
  elf_word_t size;
  elf_word_t link;
  elf_word_t info;
  elf_word_t address_alignment;
  elf_word_t entry_size;
};

size_t elf_needed_pages(const char* data, size_t size);
bool   load_elf(cap_handle_t task_cap, cap_handle_t* mem_caps, const char* data, size_t size);

#endif // LIB_ELF_H_
