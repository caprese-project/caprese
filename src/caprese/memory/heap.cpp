/**
 * @file heap.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <tuple>
#include <utility>

#include <caprese/arch/builtin_capability.h>
#include <caprese/arch/device.h>
#include <caprese/arch/memory.h>
#include <caprese/capability/bic/memory.h>
#include <caprese/memory/heap.h>
#include <caprese/task/cap.h>
#include <caprese/task/task.h>
#include <caprese/util/align.h>
#include <caprese/util/array.h>
#include <caprese/util/panic.h>

extern "C" {
  extern char _kernel_start[];
  extern char _kernel_end[];
}

namespace caprese::memory {
  namespace {
    struct free_page_t {
      mapped_address_t prev;
    };

    struct alignas(alignof(max_align_t)) using_page_block_t { };

    struct using_page_t {
      uint8_t            header[arch::PAGE_SIZE / alignof(max_align_t) / 8];
      using_page_block_t blocks[(arch::PAGE_SIZE - sizeof(header)) / alignof(max_align_t)];
    };

    constexpr size_t a = sizeof(using_page_t);
    static_assert(sizeof(using_page_t) == arch::PAGE_SIZE);

    size_t        free_page_count;
    free_page_t*  free_page_list;
    using_page_t* current_using_page;

    std::tuple<std::pair<uintptr_t, uintptr_t>, std::pair<uintptr_t, uintptr_t>> subtract_spaces(const std::pair<uintptr_t, uintptr_t>& a, const std::pair<uintptr_t, uintptr_t>& b) {
      if (a.second <= b.first || b.second <= a.first) {
        return {
          a,
          {0, 0},
        };
      }

      if (b.first <= a.first && a.second <= b.second) {
        return {
          {0, 0},
          {0, 0},
        };
      }

      if (a.first <= b.first && b.second <= a.second) {
        return {
          { a.first,  b.first},
          {b.second, a.second},
        };
      }

      if (a.first < b.first && a.second < b.second) {
        return {
          {a.first, b.first},
          {      0,       0},
        };
      }

      return {
        {b.second, a.second},
        {       0,        0},
      };
    }

    void insert(physical_address_t address, size_t size) {
      auto base = round_up(address.value, arch::PAGE_SIZE);
      size      = round_down(size, arch::PAGE_SIZE);
      for (size_t i = 0; i < size; i += arch::PAGE_SIZE) {
        uintptr_t    physical_address = base + i;
        free_page_t* next_free_page   = reinterpret_cast<free_page_t*>(CONFIG_MAPPED_SPACE_BASE + physical_address);
        next_free_page->prev          = mapped_address_t::from(free_page_list);
        free_page_list                = next_free_page;
        ++free_page_count;
      }
    }

    [[nodiscard]] mapped_address_t fetch() {
      if (free_page_list == nullptr) [[unlikely]] {
        task::task_t* kernel_task = task::get_kernel_task();
        size_t        size        = task::allocated_cap_list_size(kernel_task);
        for (task::cid_handle_t handle = 0; handle < size; ++handle) {
          if (free_page_count >= CONFIG_KERNEL_RESERVED_PAGES) [[unlikely]] {
            break;
          }

          task::cid_t* cid = task::lookup_cid(kernel_task, handle);
          if (cid == nullptr || cid->ccid != capability::bic::memory::CCID) [[unlikely]] {
            continue;
          }

          capability::capability_t* cap = task::lookup_capability(kernel_task, *cid);
          if (capability::get_field(cap, capability::bic::memory::field::VIRTUAL_ADDRESS).result != 0) {
            continue;
          }

          physical_address_t physical_address = physical_address_t::from(capability::get_field(cap, capability::bic::memory::field::PHYSICAL_ADDRESS).result);
          capability::delete_capability(cap);
          cid->ccid = 0;

          insert(physical_address, arch::PAGE_SIZE);
        }

        if (free_page_list == nullptr) [[unlikely]] {
          return mapped_address_t::null();
        }
      }
      mapped_address_t result = mapped_address_t::from(free_page_list);
      free_page_list          = free_page_list->prev.as<free_page_t>();
      --free_page_count;
      return result;
    }

    void new_current_using_page() {
      current_using_page = fetch().as<using_page_t>();
      if (current_using_page != nullptr) {
        memset(current_using_page, 0, sizeof(using_page_t::header));
        current_using_page->header[0] |= 1;
      }
    }
  } // namespace

  bool init_heap(const arch::boot_info_t* boot_info) {
    free_page_count    = 0;
    free_page_list     = nullptr;
    current_using_page = nullptr;

    auto reserved_spaces = arch::get_reserved_ram_spaces(boot_info);
    iterate_tuple(reserved_spaces, [](auto&& space) { printf("Reserved space: 0x%lx to 0x%lx\n", space.first, space.first + space.second); });

    arch::scan_device(boot_info, [&](arch::scan_callback_args_t* args) {
      if (args->flags.device) {
        return;
      }

      if (!args->flags.unavailable) {
        printf("Found an available ram(%s): address=0x%lx, size=0x%lx\n", args->device_name, args->address, args->size);

        std::pair<uintptr_t, uintptr_t> spaces[1 << std::tuple_size_v<decltype(reserved_spaces)>] = {
          {args->address, args->address + args->size}
        };
        iterate_tuple(reserved_spaces, [&spaces](auto&& reserved_space, size_t index) {
          for (size_t i = 0; i < (1ull << index); ++i) {
            if ((spaces[i].first == 0 && spaces[i].second == 0) || spaces[i].first == spaces[i].second) {
              continue;
            }
            auto&& split                = subtract_spaces(spaces[i], std::make_pair(reserved_space.first, reserved_space.first + reserved_space.second));
            spaces[i]                   = std::get<0>(split);
            spaces[i + (1ull << index)] = std::get<1>(split);
          }
        });

        for (auto& space : spaces) {
          if ((space.first == 0 && space.second == 0) || space.first == space.second) {
            continue;
          }
          printf("Inserted into kernel heap: address=0x%lx, size=0x%lx\n", space.first, space.second - space.first);
          insert(physical_address_t::from(space.first), space.second - space.first);
        }
      }
    });

    new_current_using_page();

    return current_using_page != nullptr;
  }

  mapped_address_t allocate(size_t size, size_t align) {
    if (size > arch::PAGE_SIZE) [[unlikely]] {
      panic("Too large size: 0x%lx", size);
    }
    if (size > sizeof(using_page_t::blocks)) {
      return fetch();
    }
    if (current_using_page == nullptr) [[unlikely]] {
      return mapped_address_t::null();
    }

    for (int i = array_size_of(current_using_page->header) - 1; i >= 0; --i) {
      int bit = 7;
      if (i == static_cast<int>(array_size_of(current_using_page->header)) - 1) [[unlikely]] {
        bit = 7 - array_size_of(current_using_page->header) / alignof(max_align_t);
      }
      for (; bit >= 0; --bit) {
        if (current_using_page->header[i] & (1 << bit)) {
          uintptr_t allocate_point = round_up(alignof(max_align_t) * (i * 8 + bit), align);
          uintptr_t next_point     = round_up(allocate_point + size, alignof(max_align_t));
          if (sizeof(using_page_t::blocks) > next_point) {
            current_using_page->header[next_point / alignof(max_align_t) / 8] |= 1 << (next_point / alignof(max_align_t) % 8);
            return mapped_address_t::from(reinterpret_cast<uintptr_t>(current_using_page->blocks) + allocate_point);
          } else if (sizeof(using_page_t) == next_point) {
            mapped_address_t result = mapped_address_t::from(reinterpret_cast<uintptr_t>(current_using_page->blocks) + allocate_point);
            new_current_using_page();
            return result;
          } else {
            new_current_using_page();
            return allocate(size, align);
          }
        }
      }
    }

    return mapped_address_t::null();
  }

  void deallocate(mapped_address_t addr) {
    if (addr.value & (arch::PAGE_SIZE - 1)) {
      using_page_t* using_page = reinterpret_cast<using_page_t*>(addr.value & ~(arch::PAGE_SIZE - 1));
      uintptr_t     point      = (addr.value & (arch::PAGE_SIZE - 1)) - offsetof(using_page_t, blocks);
      uintptr_t     index      = point / alignof(max_align_t) / 8;
      uintptr_t     bit        = point / alignof(max_align_t) % 8;

      if ((using_page->header[index] & (1 << bit)) == 0) [[unlikely]] {
        panic("Attempted to free an invalid pointer: %p", addr.as<void>());
      }

      using_page->header[index] &= ~(1 << bit);

      if (current_using_page != using_page) {
        for (int i = arch::PAGE_SIZE / sizeof(max_align_t) / 8 - 1; i >= 0; --i) {
          if (current_using_page->header[i]) {
            return;
          }
        }
        insert(mapped_address_t::from(using_page).physical_address(), arch::PAGE_SIZE);
      }
    } else {
      insert(addr.physical_address(), arch::PAGE_SIZE);
    }
  }

  [[nodiscard]] size_t num_remaining_pages() {
    return free_page_count;
  }
} // namespace caprese::memory
