#include <cstring>
#include <utility>

#include <kernel/user_memory.h>

namespace {
  std::pair<map_ptr<pte_t>, int> walk(map_ptr<task_t> task, uintptr_t va) {
    map_ptr<page_table_t> page_table = task->root_page_table;
    map_ptr<pte_t>        pte        = 0_map;
    int                   level      = MAX_PAGE_TABLE_LEVEL;

    for (; level >= 0; --level) {
      pte = page_table->walk(make_virt_ptr(va), level);
      if (pte->is_disabled()) {
        return std::pair<map_ptr<pte_t>, int> { 0_map, 0 };
      }

      if (!pte->is_table()) {
        break;
      }

      page_table = pte->get_next_page().as<page_table_t>();
    }

    if (pte->is_disabled() || pte->is_table() || !pte->is_user()) {
      return std::pair<map_ptr<pte_t>, int> { 0_map, 0 };
    }

    return std::pair<map_ptr<pte_t>, int> { pte, level };
  }
} // namespace

bool read_user_memory(map_ptr<task_t> task, uintptr_t src, map_ptr<void> dst, size_t size) {
  size_t read = 0;

  while (read < size) {
    auto [pte, level] = walk(task, src + read);
    if (pte == nullptr) {
      return false;
    }

    size_t page_size = get_page_size(level);
    size_t offset    = (src + read) & (page_size - 1);
    size_t length    = page_size - offset;
    if (length > size - read) {
      length = size - read;
    }

    memcpy((dst.template as<void>() + read).get(), (pte->get_next_page() + offset).get(), length);
    read += length;
  }

  return true;
}

bool write_user_memory(map_ptr<task_t> task, map_ptr<void> src, uintptr_t dst, size_t size) {
  size_t written = 0;

  while (written < size) {
    auto [pte, level] = walk(task, dst + written);
    if (pte == nullptr) {
      return false;
    }

    size_t page_size = get_page_size(level);
    size_t offset    = (dst + written) & (page_size - 1);
    size_t length    = page_size - offset;
    if (length > size - written) {
      length = size - written;
    }

    memcpy((pte->get_next_page() + offset).get(), (src.template as<void>() + written).get(), length);
    written += length;
  }

  return true;
}
