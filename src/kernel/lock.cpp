#include <bit>

#include <kernel/cls.h>
#include <kernel/lock.h>
#include <kernel/log.h>
#include <kernel/task.h>

void spinlock_t::lock() {
  while (state.test_and_set(std::memory_order_acquire)) {
    // busy waiting
  }
}

void spinlock_t::unlock() {
  state.clear(std::memory_order_release);
}

bool spinlock_t::try_lock() {
  return !state.test_and_set(std::memory_order_acquire);
}

void recursive_spinlock_t::lock() {
  uint32_t current_tid = std::bit_cast<uint32_t>(get_cls()->current_task->tid);

  if (owner == current_tid) {
    ++count;
    return;
  }

  while (true) {
    while (state.test_and_set(std::memory_order_acquire)) {
      // busy waiting
    }

    if (owner == 0) {
      owner = current_tid;
      count = 1;
      return;
    }
  }
}

void recursive_spinlock_t::unlock() {
  uint32_t current_tid = std::bit_cast<uint32_t>(get_cls()->current_task->tid);

  if (owner != current_tid) {
    panic("Invalid unlock.");
  }

  --count;
  if (count == 0) {
    owner = 0;
    state.clear(std::memory_order_release);
  }
}

bool recursive_spinlock_t::try_lock() {
  uint32_t current_tid = std::bit_cast<uint32_t>(get_cls()->current_task->tid);

  if (owner == current_tid) {
    ++count;
    return true;
  }

  if (state.test_and_set(std::memory_order_acquire)) {
    return false;
  }

  if (owner == 0) {
    owner = current_tid;
    count = 1;
  }

  return true;
}
