#include <cstdlib>
#include <cstring>

#include <runtime/memory.h>
#include <runtime/task.h>
#include <runtime/types.h>

extern "C" {
  using __atexit_callback_t = void (*)();

  __atexit_callback_t  __atexit_callbacks[32];
  __atexit_callback_t* __extended_atexit_callbacks;

  __atexit_callback_t  __at_quick_exit_callbacks[32];
  __atexit_callback_t* __extended_at_quick_exit_callbacks;

  int __atexit_callback_count;
  int __at_quick_exit_callback_count;

  extern task_handle_t __init_task_handle;
  extern task_handle_t __this_task_handle;

  void __init_exit_procs() noexcept {
    for (int i = 0; i < 32; ++i) {
      __atexit_callbacks[i]        = nullptr;
      __at_quick_exit_callbacks[i] = nullptr;
    }
    __extended_atexit_callbacks        = nullptr;
    __extended_at_quick_exit_callbacks = nullptr;

    __atexit_callback_count        = 0;
    __at_quick_exit_callback_count = 0;
  }

  [[noreturn]] void abort() noexcept {
    _Exit(EXIT_FAILURE);
  }

  [[noreturn]] void _Exit(int status) noexcept {
    while (true) {
      kill_task(__this_task_handle, status);
      asm volatile("wfi");
    }
  }

  [[noreturn]] void exit(int status) {
    while (__atexit_callback_count > 32) {
      __extended_atexit_callbacks[--__atexit_callback_count]();
    }
    while (__atexit_callback_count > 0) {
      __atexit_callbacks[--__atexit_callback_count]();
    }
    _Exit(status);
  }

  int atexit(void (*func)()) noexcept {
    if (__atexit_callback_count < 32) {
      __atexit_callbacks[__atexit_callback_count] = func;
    } else {
      __atexit_callback_t* new_atexit_callbacks = static_cast<__atexit_callback_t*>(realloc(__extended_atexit_callbacks, __atexit_callback_count - 32 + 1));
      if (new_atexit_callbacks == nullptr) [[unlikely]] {
        return 1;
      }
      __extended_atexit_callbacks                               = new_atexit_callbacks;
      __extended_atexit_callbacks[__atexit_callback_count - 32] = func;
    }
    ++__atexit_callback_count;
    return 0;
  }

  [[noreturn]] void quick_exit(int status) noexcept {
    while (__at_quick_exit_callback_count > 32) {
      __extended_at_quick_exit_callbacks[--__at_quick_exit_callback_count]();
    }
    while (__at_quick_exit_callback_count > 0) {
      __at_quick_exit_callbacks[--__at_quick_exit_callback_count]();
    }
    _Exit(status);
  }

  int at_quick_exit(void (*func)()) noexcept {
    if (__at_quick_exit_callback_count < 32) {
      __at_quick_exit_callbacks[__at_quick_exit_callback_count] = func;
    } else {
      __atexit_callback_t* new_at_quick_exit_callbacks = static_cast<__atexit_callback_t*>(realloc(__extended_at_quick_exit_callbacks, __at_quick_exit_callback_count - 32 + 1));
      if (new_at_quick_exit_callbacks == nullptr) [[unlikely]] {
        return 1;
      }
      __extended_at_quick_exit_callbacks                                      = new_at_quick_exit_callbacks;
      __extended_at_quick_exit_callbacks[__at_quick_exit_callback_count - 32] = func;
    }
    ++__at_quick_exit_callback_count;
    return 0;
  }

  void* aligned_alloc([[maybe_unused]] size_t alignment, [[maybe_unused]] size_t size) noexcept {
    memory_handle_t handle = fetch_memory_handle(MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE);
    if (handle == 0) [[unlikely]] {
      return nullptr;
    }
    return reinterpret_cast<void*>(get_virtual_address(handle));
  }

  void* malloc(size_t size) noexcept {
    return aligned_alloc(0, size);
  }

  void* calloc(size_t num, size_t size) noexcept {
    void* ptr = malloc(size * num);
    if (ptr == nullptr) [[unlikely]] {
      return nullptr;
    }
    memset(ptr, 0, size * num);
    return ptr;
  }

  void* realloc(void* ptr, size_t size) noexcept {
    (void)ptr;
    (void)size;
    // TODO: impl
    return nullptr;
  }

  void free(void* ptr) noexcept {
    (void)ptr;
    // TODO: impl
  }
}
