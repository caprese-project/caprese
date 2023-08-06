#include <cstdlib>

#include <caprese/memory/heap.h>

extern "C" {
  void* malloc(size_t size) {
    return caprese::memory::allocate(size, 0).as<void>();
  }

  void* calloc(size_t n, size_t size) {
    void* ptr = malloc(n * size);
    if (ptr == nullptr) [[unlikely]] {
      return nullptr;
    }

    char* cptr = reinterpret_cast<char*>(ptr);
    for (size_t i = 0; i < n * size; ++i) {
      cptr[i] = '\0';
    }

    return ptr;
  }

  void* aligned_alloc(size_t alignment, size_t size) {
    return caprese::memory::allocate(size, alignment).as<void>();
  }

  void free(void* ptr) {
    caprese::memory::deallocate(caprese::memory::mapped_address_t::from(ptr));
  }
}
