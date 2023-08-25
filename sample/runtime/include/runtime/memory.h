#ifndef RUNTIME_MEMORY_H_
#define RUNTIME_MEMORY_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define MEMORY_FLAG_READABLE   (1 << 0)
#define MEMORY_FLAG_WRITABLE   (1 << 1)
#define MEMORY_FLAG_EXECUTABLE (1 << 2)

  uintptr_t get_physical_address(memory_handle_t handle);
  uintptr_t get_virtual_address(memory_handle_t handle);
  tid_t     get_mapped_tid(memory_handle_t handle);
  bool      is_mapped(memory_handle_t handle);

  bool map_memory_page_to_task(task_handle_t task_handle, memory_handle_t memory_handle, uintptr_t virtual_address, unsigned flags);
  bool map_memory_page(memory_handle_t handle, uintptr_t virtual_address, unsigned flags);
  bool unmap_memory_page(memory_handle_t handle);
  bool read_memory(memory_handle_t handle, void* dst, size_t offset, size_t size);
  bool write_memory(memory_handle_t handle, const void* src, size_t offset, size_t size);

  memory_handle_t fetch_memory_handle(unsigned flags);
  void*           map_memory_page_to_heap(memory_handle_t handle, unsigned flags);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_MEMORY_H_
