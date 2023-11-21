#ifndef KERNEL_LOCK_H_
#define KERNEL_LOCK_H_

#include <atomic>

struct spinlock_t {
  std::atomic_flag state;

  spinlock_t(): state(false) { }

  void lock();
  void unlock();
  bool try_lock();
};

struct recursive_spinlock_t {
  std::atomic_flag state;
  uint32_t         owner;
  uint32_t         count;

  recursive_spinlock_t(): state(false), owner(0), count(0) { }

  void lock();
  void unlock();
  bool try_lock();
};

#endif // KERNEL_LOCK_H_
