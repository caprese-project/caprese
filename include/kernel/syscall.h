#ifndef KERNEL_SYSCALL_H_
#define KERNEL_SYSCALL_H_

#include <cerrno>

#include <kernel/task.h>
#include <libcaprese/syscall.h>

struct syscall_args_t {
  uintptr_t args[7];
  uintptr_t code;
};

void get_syscall_args(map_ptr<syscall_args_t> args);

constexpr sysret_t sysret_s_ok(uintptr_t result) {
  return sysret_t { result, SYS_S_OK };
}

constexpr sysret_t sysret_e_unknown() {
  return sysret_t { 0, SYS_E_UNKNOWN };
}

constexpr sysret_t sysret_e_cap_type() {
  return sysret_t { 0, SYS_E_CAP_TYPE };
}

constexpr sysret_t sysret_e_cap_state() {
  return sysret_t { 0, SYS_E_CAP_STATE };
}

constexpr sysret_t sysret_e_ill_code() {
  return sysret_t { 0, SYS_E_ILL_CODE };
}

constexpr sysret_t sysret_e_ill_args() {
  return sysret_t { 0, SYS_E_ILL_ARGS };
}

constexpr sysret_t sysret_e_ill_state() {
  return sysret_t { 0, SYS_E_ILL_STATE };
}

constexpr sysret_t sysret_e_out_of_cap_space() {
  return sysret_t { 0, SYS_E_OUT_OF_CAP_SPACE };
}

inline sysret_t errno_to_sysret() {
  return sysret_t { 0, errno };
}

constexpr inline const char* sysret_error_to_str(sysret_error_t err) {
  switch (err) {
    case SYS_S_OK:
      return "SYS_S_OK";
    case SYS_E_UNKNOWN:
      return "SYS_E_UNKNOWN";
    case SYS_E_CAP_TYPE:
      return "SYS_E_CAP_TYPE";
    case SYS_E_CAP_STATE:
      return "SYS_E_CAP_STATE";
    case SYS_E_ILL_CODE:
      return "SYS_E_ILL_CODE";
    case SYS_E_ILL_ARGS:
      return "SYS_E_ILL_ARGS";
    case SYS_E_ILL_STATE:
      return "SYS_E_ILL_STATE";
    case SYS_E_CANCELED:
      return "SYS_E_CANCELED";
    case SYS_E_BLOCKED:
      return "SYS_E_BLOCKED";
    case SYS_E_OUT_OF_CAP_SPACE:
      return "SYS_E_OUT_OF_CAP_SPACE";
    default:
      return "<UNKNOWN CODE>";
  }
}

sysret_t invoke_syscall();
sysret_t invoke_syscall_system(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_arch(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_mem_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_task_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_endpoint_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_page_table_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_virt_page_cap(uint16_t id, map_ptr<syscall_args_t> args);
sysret_t invoke_syscall_id_cap(uint16_t id, map_ptr<syscall_args_t> args);

#endif // KERNEL_SYSCALL_H_
