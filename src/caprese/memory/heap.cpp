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

#include <caprese/arch/device.h>
#include <caprese/arch/memory.h>
#include <caprese/memory/heap.h>
#include <caprese/util/align.h>

extern "C" {
  extern char _kernel_start[];
  extern char _kernel_end[];
}

namespace caprese::memory {
  namespace {
    struct free_page_t {
      mapped_address_t prev;
    };

    struct using_page_t {
      uint8_t     used_flags[round_up(arch::PAGE_SIZE / sizeof(max_align_t) / 8, sizeof(max_align_t))];
      max_align_t block[(arch::PAGE_SIZE - sizeof(used_flags)) / sizeof(max_align_t)];
    };

    constexpr size_t a = sizeof(using_page_t);
    static_assert(sizeof(using_page_t) == arch::PAGE_SIZE);

    free_page_t*  free_page_list;
    using_page_t* current_using_page;

    std::tuple<std::pair<uintptr_t, uintptr_t>, std::pair<uintptr_t, uintptr_t>> subtract_spaces(const std::pair<uintptr_t, uintptr_t>& a,
                                                                                                 const std::pair<uintptr_t, uintptr_t>& b) {
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
      }
    }

    void new_current_using_page() {
      current_using_page = reinterpret_cast<using_page_t*>(free_page_list);
      if (current_using_page) {
        free_page_list = free_page_list->prev.as<free_page_t>();
        memset(current_using_page, 0, sizeof(using_page_t));
        current_using_page->used_flags[0] |= 1;
      }
    }
  } // namespace

  void init_heap_space(arch::boot_info_t* boot_info) {
    free_page_list     = nullptr;
    current_using_page = nullptr;

    uintptr_t reserved_address_start = -1;
    size_t    reserved_address_end   = 0;

    arch::scan_device(boot_info, [&](arch::scan_callback_args_t* args) {
      if (args->flags.device) {
        return;
      }

#if defined(CONFIG_ARCH_RISCV)
      if (strncmp(args->device_name, "mmode_resv", 10) == 0) {
        reserved_address_start = std::min(reserved_address_start, args->address);
        reserved_address_end   = std::max(reserved_address_end, args->address + args->size);
      }
#endif // defined(CONFIG_ARCH_RISCV)

      if (!args->flags.unavailable) {
        printf("Found an available ram(%s): address=0x%lx, size=0x%lx\n", args->device_name, args->address, args->size);

        auto spaces =
            subtract_spaces({ args->address, args->address + args->size },
                            { mapped_address_t::from(_kernel_start).physical_address().value, mapped_address_t::from(_kernel_end).physical_address().value });
        iterate_tuple(spaces, [&](const std::pair<uintptr_t, uintptr_t>& space) {
          if ((space.first == 0 && space.second == 0) || space.first == space.second) {
            return;
          }

          auto spaces = subtract_spaces(space, { reserved_address_start, reserved_address_end });
          iterate_tuple(spaces, [](const std::pair<uintptr_t, uintptr_t>& space) {
            if ((space.first == 0 && space.second == 0) || space.first == space.second) {
              return;
            }

            printf("Inserted into kernel heap: address=0x%lx, size=0x%lx\n", space.first, space.second - space.first);
            insert(physical_address_t::from(space.first), space.second - space.first);
          });
        });
      }
    });

    new_current_using_page();
  }

  mapped_address_t allocate(size_t size, size_t align) {
    if (size > sizeof(using_page_t::block)) {
      if (free_page_list == nullptr) [[unlikely]] {
        return mapped_address_t::null();
      }
      mapped_address_t result = mapped_address_t::from(free_page_list);
      free_page_list          = free_page_list->prev.as<free_page_t>();
      return result;
    }

    for (int i = arch::PAGE_SIZE / sizeof(max_align_t) / 8 - 1; i >= 0; --i) {
      for (int bit = 7; bit >= 0; --bit) {
        if (current_using_page->used_flags[i] & (1 << bit)) {
          uintptr_t allocate_point = round_up(sizeof(max_align_t) * (i * 8 + bit), align);
          uintptr_t next_point     = allocate_point + size;
          if (sizeof(using_page_t::block) > next_point) {
            current_using_page->used_flags[next_point / sizeof(using_page_t) / 8] |= 1 << (next_point % 8);
            return mapped_address_t::from(reinterpret_cast<uintptr_t>(current_using_page->block) + allocate_point);
          } else if (sizeof(using_page_t) == next_point) {
            mapped_address_t result = mapped_address_t::from(reinterpret_cast<uintptr_t>(current_using_page->block) + allocate_point);
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
      uintptr_t     point      = addr.value & (arch::PAGE_SIZE - 1);
      uintptr_t     index      = point / sizeof(using_page_t) / 8;

      if ((using_page->used_flags[index] & (1 << (point % 8))) == 0) [[unlikely]] {
        return; // TODO: panic()
      }

      using_page->used_flags[index] &= ~(1 << (point % 8));

      if (current_using_page != using_page) {
        for (int i = arch::PAGE_SIZE / sizeof(max_align_t) / 8 - 1; i >= 0; --i) {
          if (current_using_page->used_flags[i]) {
            return;
          }
        }
        insert(mapped_address_t::from(using_page).physical_address(), arch::PAGE_SIZE);
      }
    } else {
      insert(addr.physical_address(), arch::PAGE_SIZE);
    }
  }
} // namespace caprese::memory
