#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <iterator>

#include <kernel/align.h>
#include <kernel/arch/dtb.h>
#include <kernel/log.h>

namespace {
  uint32_t _dtb_to_u32(map_ptr<char> ptr) {
    return std::byteswap(*ptr.as<uint32_t>());
  }

  uint64_t _dtb_to_u64(map_ptr<char> ptr) {
    return std::byteswap(*ptr.as<uint32_t>());
  }

  uint32_t _dtb_next_u32(map_ptr<char>* ptr) {
    uint32_t result = _dtb_to_u32(*ptr);
    *ptr += sizeof(uint32_t);
    return result;
  }

  const char* _dtb_next_str(map_ptr<char>* ptr) {
    const char* result = ptr->get();
    *ptr += strlen(result) + 1;
    return result;
  }

  void _dtb_align(map_ptr<char>* ptr) {
    *ptr = make_map_ptr(round_up(ptr->raw(), sizeof(uint32_t)));
  }

  void _dtb_skip_nop(map_ptr<char>* ptr) {
    while (_dtb_to_u32(*ptr) == FDT_NOP) {
      *ptr += sizeof(uint32_t);
    }
  }

  void _parse_dtb_node(map_ptr<char>* src, map_ptr<char> end, map_ptr<fdt_header_t> header, uint32_t address_cells, uint32_t size_cells, bool (*callback)(map_ptr<dtb_node_t>)) {
    assert(src != nullptr);
    assert(*src != nullptr);
    assert(end != nullptr);
    assert(header != nullptr);
    assert(address_cells > 0);
    assert(callback != nullptr);
    assert(src->raw() % sizeof(uint32_t) == 0);

    map_ptr<char> str_start = header.as<char>() + std::byteswap(header->off_dt_strings);
    map_ptr<char> str_end   = str_start + std::byteswap(header->size_dt_strings);

    uint32_t token = _dtb_next_u32(src);
    if (token != FDT_BEGIN_NODE) [[unlikely]] {
      loge("arch/dtb", "Invalid FDT token: %x", token);
      return;
    }

    dtb_node_t node {
      ._next         = 0_map,
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

    bool should_continue = callback(make_map_ptr(&node));

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

      const char* name = (str_start + nameoff).get();
      if (name >= str_end.get()) [[unlikely]] {
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
        _parse_dtb_node(src, end, header, child_address_cells, child_size_cells, []([[maybe_unused]] map_ptr<dtb_node_t>) { return false; });
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

void for_each_dtb_node(map_ptr<char> dtb, bool (*callback)(map_ptr<dtb_node_t>)) {
  assert(dtb != nullptr);
  assert(callback != nullptr);

  map_ptr<fdt_header_t> header = dtb.as<fdt_header_t>();
  if (header->magic != std::byteswap(FDT_HEADER_MAGIC)) [[unlikely]] {
    loge("arch/dtb", "Invalid FDT magic: %x", header->magic);
    return;
  }

  map_ptr<char> src = dtb + std::byteswap(header->off_dt_struct);
  map_ptr<char> end = src + std::byteswap(header->size_dt_struct);

  _dtb_skip_nop(&src);

  _parse_dtb_node(&src, end, header, 2, 1, callback);
}

void for_each_dtb_prop(map_ptr<dtb_node_t> node, bool (*callback)(map_ptr<dtb_node_t>, map_ptr<dtb_prop_t>)) {
  assert(node != nullptr);
  assert(callback != nullptr);

  map_ptr<char> end       = node->header.as<char>() + std::byteswap(node->header->off_dt_struct) + std::byteswap(node->header->size_dt_struct);
  map_ptr<char> str_start = node->header.as<char>() + std::byteswap(node->header->off_dt_strings);
  map_ptr<char> str_end   = str_start + std::byteswap(node->header->size_dt_strings);

  map_ptr<char> src = node->_next;

  while (src < end) {
    _dtb_skip_nop(&src);

    if (_dtb_to_u32(src) != FDT_PROP) [[unlikely]] {
      break;
    }

    src += sizeof(uint32_t); // token
    uint32_t len     = _dtb_next_u32(&src);
    uint32_t nameoff = _dtb_next_u32(&src);

    const char* name = (str_start + nameoff).get();
    if (name >= str_end.get()) [[unlikely]] {
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
            prop.str = src.get();
            break;
          case array:
            prop.array.length = len;
            prop.array.data   = src.as<uint8_t>().get();
            break;
          case phandle:
            prop.phandle = _dtb_to_u32(src);
            break;
          case str_list:
            prop.str_list.length = len;
            prop.str_list.data   = src.get();
            break;
          case unknown:
            break;
        }
      }

      if (!callback(node, make_map_ptr(&prop))) {
        break;
      }
    }

    src += len;
    _dtb_align(&src);
  }
}
