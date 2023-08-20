#include <caprese/task/task.h>

#include <lib/capability.h>
#include <lib/syscall.h>

uint16_t get_cap_type(cap_handle_t cap_handle) {
  auto [base, base_error] = sys_cap_list_base();
  auto [size, size_error] = sys_cap_list_size();

  if (base_error || size_error) [[unlikely]] {
    return 0;
  }

  if (cap_handle >= size) [[unlikely]] {
    return 0;
  }

  return reinterpret_cast<caprese::task::cid_t*>(base)[cap_handle].ccid;
}
