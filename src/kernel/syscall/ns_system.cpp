#include <iterator>

#include <kernel/cap_space.h>
#include <kernel/core_id.h>
#include <kernel/log.h>
#include <kernel/page.h>
#include <kernel/syscall/ns_system.h>
#include <kernel/task.h>

namespace {
  constexpr const char* tag = "syscall/system";
} // namespace

sysret_t invoke_sys_system_null(map_ptr<syscall_args_t>) {
  return sysret_s_ok(0);
}

sysret_t invoke_sys_system_core_id(map_ptr<syscall_args_t>) {
  return sysret_s_ok(get_core_id());
}

sysret_t invoke_sys_system_page_size(map_ptr<syscall_args_t>) {
  return sysret_s_ok(PAGE_SIZE);
}

sysret_t invoke_sys_system_user_space_start(map_ptr<syscall_args_t>) {
  return sysret_s_ok(CONFIG_USER_SPACE_BASE);
}

sysret_t invoke_sys_system_user_space_end(map_ptr<syscall_args_t>) {
  return sysret_s_ok(CONFIG_USER_SPACE_BASE + CONFIG_USER_SPACE_SIZE);
}

sysret_t invoke_sys_system_caps_per_cap_space(map_ptr<syscall_args_t>) {
  return sysret_s_ok(std::size(static_cast<cap_space_t*>(nullptr)->slots));
}

sysret_t invoke_sys_system_yield(map_ptr<syscall_args_t>) {
  yield();
  return sysret_s_ok(0);
}

sysret_t invoke_sys_system_cap_size(map_ptr<syscall_args_t> args) {
  if (args->args[0] > CAP_ZOMBIE) {
    loge(tag, "Invalid cap type: %d", args->args[0]);
    return sysret_e_ill_args();
  }
  return sysret_s_ok(get_cap_size(static_cast<cap_type_t>(args->args[0])));
}

sysret_t invoke_sys_system_cap_align(map_ptr<syscall_args_t> args) {
  if (args->args[0] > CAP_ZOMBIE) {
    loge(tag, "Invalid cap type: %d", args->args[0]);
    return sysret_e_ill_args();
  }
  return sysret_s_ok(get_cap_align(static_cast<cap_type_t>(args->args[0])));
}
