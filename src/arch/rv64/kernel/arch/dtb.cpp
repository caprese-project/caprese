#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <iterator>

#include <kernel/align.h>
#include <kernel/arch/dtb.h>
#include <log/log.h>

namespace {
  uint32_t _dtb_to_u32(const char* ptr) {
    return std::byteswap(*reinterpret_cast<const uint32_t*>(ptr));
  }

  uint64_t _dtb_to_u64(const char* ptr) {
    return std::byteswap(*reinterpret_cast<const uint64_t*>(ptr));
  }

  uint32_t _dtb_next_u32(const char** ptr) {
    uint32_t result = _dtb_to_u32(*ptr);
    *ptr += sizeof(uint32_t);
    return result;
  }

  const char* _dtb_next_str(const char** ptr) {
    const char* result = *ptr;
    *ptr += strlen(result) + 1;
    return result;
  }

  void _dtb_align(const char** ptr) {
    *ptr = reinterpret_cast<const char*>(round_up(reinterpret_cast<uintptr_t>(*ptr), sizeof(uint32_t)));
  }

  void _dtb_skip_nop(const char** ptr) {
    while (_dtb_to_u32(*ptr) == FDT_NOP) {
      *ptr += sizeof(uint32_t);
    }
  }

  void _parse_dtb_node(const char** src, const char* end, const fdt_header_t* header, uint32_t address_cells, uint32_t size_cells, bool (*callback)(const dtb_node_t*)) {
    assert(src != nullptr);
    assert(*src != nullptr);
    assert(end != nullptr);
    assert(header != nullptr);
    assert(address_cells > 0);
    assert(callback != nullptr);
    assert(reinterpret_cast<uintptr_t>(*src) % sizeof(uint32_t) == 0);

    const char* str_start = reinterpret_cast<const char*>(header) + std::byteswap(header->off_dt_strings);
    const char* str_end   = str_start + std::byteswap(header->size_dt_strings);

    uint32_t token = _dtb_next_u32(src);
    if (token != FDT_BEGIN_NODE) [[unlikely]] {
      loge("arch/dtb", "Invalid FDT token: %x", token);
      return;
    }

    dtb_node_t node {
      ._next         = nullptr,
      .unit_address  = 0,
      .header        = header,
      .address_cells = address_cells,
      .size_cells    = size_cells,
      .name          = {},
    };

    const char* name   = _dtb_next_str(src);
    const char* at_pos = strchr(name, '@');

    if (at_pos != nullptr) [[likely]] {
      node.unit_address = static_cast<uintptr_t>(strtoull(at_pos + 1, nullptr, 16));
    } else {
      at_pos = name + strlen(name);
    }

    size_t name_len = at_pos - name;
    if (name_len >= std::size(node.name)) [[unlikely]] {
      logw("arch/dtb", "Node name too long: %s", name);
    }

    strncpy(node.name, name, std::min(name_len, std::size(node.name) - 1));

    _dtb_align(src);
    _dtb_skip_nop(src);

    node._next = *src;

    bool should_continue = callback(&node);

    uint32_t child_address_cells = 2;
    uint32_t child_size_cells    = 1;

    while (*src < end) {
      _dtb_skip_nop(src);

      if (_dtb_to_u32(*src) != FDT_PROP) [[unlikely]] {
        break;
      }

      *src += sizeof(uint32_t); // token
      uint32_t len     = _dtb_next_u32(src);
      uint32_t nameoff = _dtb_next_u32(src);

      const char* name = str_start + nameoff;
      if (name >= str_end) [[unlikely]] {
        loge("arch/dtb", "Invalid FDT string offset: %x", nameoff);
      } else if (strcmp(name, "#address-cells") == 0) [[unlikely]] {
        child_address_cells = _dtb_to_u32(*src);
      } else if (strcmp(name, "#size-cells") == 0) [[unlikely]] {
        child_size_cells = _dtb_to_u32(*src);
      }

      *src += len;
      _dtb_align(src);
    }

    while (*src < end) {
      _dtb_skip_nop(src);

      if (_dtb_to_u32(*src) != FDT_BEGIN_NODE) [[unlikely]] {
        break;
      }

      if (should_continue) {
        _parse_dtb_node(src, end, header, child_address_cells, child_size_cells, callback);
      } else {
        _parse_dtb_node(src, end, header, child_address_cells, child_size_cells, []([[maybe_unused]] const dtb_node_t*) { return false; });
      }
    }

    if (*src >= end) [[unlikely]] {
      loge("arch/dtb", "Unexpected end of FDT");
    } else if (_dtb_to_u32(*src) != FDT_END_NODE) [[unlikely]] {
      loge("arch/dtb", "Invalid FDT token: %x", _dtb_to_u32(*src));
    } else {
      *src += sizeof(uint32_t);
    }
  }

} // namespace

dtb_prop_type_t find_prop_type(const char* name) {
  constexpr auto compare = [](const char* a, const char* b) {
    return strcmp(a, b) < 0;
  };

  if (std::binary_search(FDT_U32_TYPES, FDT_U32_TYPES + std::size(FDT_U32_TYPES), name, compare)) {
    return dtb_prop_type_t::u32;
  }
  if (std::binary_search(FDT_U64_TYPES, FDT_U64_TYPES + std::size(FDT_U64_TYPES), name, compare)) {
    return dtb_prop_type_t::u64;
  }
  if (std::binary_search(FDT_STR_TYPES, FDT_STR_TYPES + std::size(FDT_STR_TYPES), name, compare)) {
    return dtb_prop_type_t::str;
  }
  if (std::binary_search(FDT_ARRAY_TYPES, FDT_ARRAY_TYPES + std::size(FDT_ARRAY_TYPES), name, compare)) {
    return dtb_prop_type_t::array;
  }
  if (std::binary_search(FDT_PHANDLE_TYPES, FDT_PHANDLE_TYPES + std::size(FDT_PHANDLE_TYPES), name, compare)) {
    return dtb_prop_type_t::phandle;
  }
  if (std::binary_search(FDT_STR_LIST_TYPES, FDT_STR_LIST_TYPES + std::size(FDT_STR_LIST_TYPES), name, compare)) {
    return dtb_prop_type_t::str_list;
  }

  return dtb_prop_type_t::unknown;
}

void for_each_dtb_node(map_addr_t dtb, bool (*callback)(const dtb_node_t*)) {
  assert(dtb != nullptr);
  assert(callback != nullptr);

  const fdt_header_t* header = dtb.as<const fdt_header_t*>();
  if (header->magic != std::byteswap(FDT_HEADER_MAGIC)) [[unlikely]] {
    loge("arch/dtb", "Invalid FDT magic: %x", header->magic);
    return;
  }

  const char* src = dtb.as<const char*>() + std::byteswap(header->off_dt_struct);
  const char* end = src + std::byteswap(header->size_dt_struct);

  _dtb_skip_nop(&src);

  _parse_dtb_node(&src, end, header, 2, 1, callback);
}

void for_each_dtb_prop(const dtb_node_t* node, bool (*callback)(const dtb_node_t*, const dtb_prop_t*)) {
  assert(node != nullptr);
  assert(callback != nullptr);

  const char* end       = reinterpret_cast<const char*>(node->header) + std::byteswap(node->header->off_dt_struct) + std::byteswap(node->header->size_dt_struct);
  const char* str_start = reinterpret_cast<const char*>(node->header) + std::byteswap(node->header->off_dt_strings);
  const char* str_end   = str_start + std::byteswap(node->header->size_dt_strings);

  const char* src = node->_next;

  while (src < end) {
    _dtb_skip_nop(&src);

    if (_dtb_to_u32(src) != FDT_PROP) [[unlikely]] {
      break;
    }

    src += sizeof(uint32_t); // token
    uint32_t len     = _dtb_next_u32(&src);
    uint32_t nameoff = _dtb_next_u32(&src);

    const char* name = str_start + nameoff;
    if (name >= str_end) [[unlikely]] {
      loge("arch/dtb", "Invalid FDT string offset: %x", nameoff);
    } else {
      dtb_prop_t prop;
      prop.name = name;
      prop.type = dtb_prop_type_t::unknown;

      if (len == 0) {
        prop.type = dtb_prop_type_t::empty;
      } else {
        prop.type = find_prop_type(name);
        switch (prop.type) {
          using enum dtb_prop_type_t;
          case empty:
            break;
          case u32:
            prop.u32 = _dtb_to_u32(src);
            break;
          case u64:
            prop.u64 = _dtb_to_u64(src);
            break;
          case str:
            prop.str = src;
            break;
          case array:
            prop.array.length = len;
            prop.array.data   = reinterpret_cast<const uint8_t*>(src);
            break;
          case phandle:
            prop.phandle = _dtb_to_u32(src);
            break;
          case str_list:
            prop.str_list.length = len;
            prop.str_list.data   = src;
            break;
          case unknown:
            break;
        }
      }

      if (!callback(node, &prop)) {
        break;
      }
    }

    src += len;
    _dtb_align(&src);
  }
}
