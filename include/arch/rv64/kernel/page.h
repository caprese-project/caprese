#ifndef ARCH_RV64_KERNEL_PAGE_H_
#define ARCH_RV64_KERNEL_PAGE_H_

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <kernel/address.h>

constexpr size_t PAGE_SIZE_BIT        = 12;
constexpr size_t PAGE_SIZE            = 1 << PAGE_SIZE_BIT;
constexpr size_t NUM_PAGE_TABLE_ENTRY = PAGE_SIZE / sizeof(uint64_t);

#if defined(CONFIG_MMU_SV39)
constexpr size_t NUM_PAGE_TABLE_LEVEL  = 3;
constexpr size_t KILO_PAGE_TABLE_LEVEL = 0;
constexpr size_t MEGA_PAGE_TABLE_LEVEL = 1;
constexpr size_t GIGA_PAGE_TABLE_LEVEL = 2;
constexpr size_t MAX_PAGE_TABLE_LEVEL  = GIGA_PAGE_TABLE_LEVEL;
#elif defined(CONFIG_MMU_SV48)
constexpr size_t NUM_PAGE_TABLE_LEVEL  = 4;
constexpr size_t KILO_PAGE_TABLE_LEVEL = 0;
constexpr size_t MEGA_PAGE_TABLE_LEVEL = 1;
constexpr size_t GIGA_PAGE_TABLE_LEVEL = 2;
constexpr size_t TERA_PAGE_TABLE_LEVEL = 3;
constexpr size_t MAX_PAGE_TABLE_LEVEL  = TERA_PAGE_TABLE_LEVEL;
#endif

constexpr size_t get_page_size(size_t level) {
  return PAGE_SIZE << (9 * level);
}

constexpr size_t get_page_size_bit(size_t level) {
  return PAGE_SIZE_BIT + (9 * level);
}

struct pte_flags_t {
  uint64_t readable  : 1;
  uint64_t writable  : 1;
  uint64_t executable: 1;
  uint64_t user      : 1;
  uint64_t global    : 1;
};

struct pte_t {
  uint64_t v               : 1;
  uint64_t r               : 1;
  uint64_t w               : 1;
  uint64_t x               : 1;
  uint64_t u               : 1;
  uint64_t g               : 1;
  uint64_t a               : 1;
  uint64_t d               : 1;
  uint64_t rsv             : 2;
  uint64_t next_page_number: 44;

  inline void enable() {
    this->v = 1;
  }

  inline void disable() {
    this->v = 0;
  }

  inline bool is_enabled() const {
    return this->v;
  }

  inline bool is_disabled() const {
    return !this->v;
  }

  inline bool is_table() const {
    return this->v && !this->r && !this->w && !this->x && !this->u && !this->g;
  }

  inline void set_flags(pte_flags_t flags) {
    this->r = flags.readable;
    this->w = flags.writable;
    this->x = flags.executable;
    this->u = flags.user;
    this->g = flags.global;
  }

  inline void set_next_page(map_ptr<void> next_page) {
    this->next_page_number = next_page.as_phys().raw() >> PAGE_SIZE_BIT;
  }

  [[nodiscard]] inline map_ptr<void> get_next_page() {
    return phys_ptr<void>::from(this->next_page_number << PAGE_SIZE_BIT);
  }
};

struct alignas(PAGE_SIZE) page_table_t {
  pte_t entries[NUM_PAGE_TABLE_ENTRY];

  inline map_ptr<pte_t> walk(virt_ptr<void> va, size_t level) {
    assert(va.raw() < CONFIG_MAX_VIRTUAL_ADDRESS);
    assert(level < NUM_PAGE_TABLE_LEVEL);
    size_t index = (va.raw() >> (9 * level + PAGE_SIZE_BIT)) & 0x1ff;
    return make_map_ptr(&entries[index]);
  }
};

static_assert(sizeof(page_table_t) == PAGE_SIZE);

#endif // ARCH_RV64_KERNEL_PAGE_H_
