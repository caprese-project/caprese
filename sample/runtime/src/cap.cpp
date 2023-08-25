#include <bit>

#include <caprese/task/cap.h>

#include <runtime/cap.h>
#include <runtime/global.h>
#include <runtime/syscall.h>

extern "C" {
  uint16_t get_handle_type(handle_t handle) {
    sysret_t sysret = sys_cap_list_size();
    if (sysret.error) [[unlikely]] {
      return 0;
    }

    return std::bit_cast<caprese::task::cid_t>(__handle_list_base[handle]).ccid;
  }
}
