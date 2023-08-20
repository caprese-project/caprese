#include <cstdlib>

#include <caprese/capability/bic/task.h>

namespace caprese::capability::bic::task {
  class_t* create_class() {
    class_t* cap_class = create_capability_class();
    if (cap_class == nullptr) [[unlikely]] {
      return nullptr;
    }
    if (make_ccid(cap_class) != CCID) [[unlikely]] {
      return nullptr;
    }

    cap_class->name            = "memory";
    cap_class->flags           = CLASS_FLAG_VALID | CLASS_FLAG_BUILTIN | CLASS_FLAG_MOVABLE | CLASS_FLAG_COPYABLE;
    cap_class->num_permissions = 2;
    cap_class->num_fields      = 1;
    cap_class->num_methods     = 5;
    cap_class->methods         = static_cast<method_t*>(malloc(cap_class->num_methods * sizeof(method_t)));

    if (cap_class->methods == nullptr) [[unlikely]] {
      return nullptr;
    }

    cap_class->methods[method::STATE]        = method_state;
    cap_class->methods[method::SWITCH]       = method_switch;
    cap_class->methods[method::SET_REGISTER] = method_set_register;
    cap_class->methods[method::ALIVE]        = method_alive;
    cap_class->methods[method::KILL]         = method_kill;

    return cap_class;
  }

  capability_t* create(caprese::task::tid_t tid) {
    capability_t* cap = create_capability(CCID);
    if (cap == nullptr) [[unlikely]] {
      return nullptr;
    }

    set_permission(cap, permission::SWITCHABLE, true);
    set_permission(cap, permission::KILLABLE, true);
    set_field(cap, field::TID, std::bit_cast<uint32_t>(tid));

    return cap;
  }

  cap_ret_t method_state(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    return { .result = task->state, .error = 0 };
  }

  cap_ret_t method_switch(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;

    auto [switchable, switchable_error] = is_permitted(cap, permission::SWITCHABLE);
    if (switchable_error || !switchable) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (task->state != TASK_STATE_READY && task->state != TASK_STATE_CREATING) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    switch_to(task);

    return { .result = 0, .error = 0 };
  }

  cap_ret_t method_set_register(capability_t* cap, uintptr_t reg, uintptr_t value, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    if (task->state != TASK_STATE_CREATING) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    arch::set_register(&task->arch_task, reg, value);

    return { .result = 0, .error = 0 };
  }

  cap_ret_t method_alive(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    return { .result = task->state != TASK_STATE_UNUSED, .error = 0 };
  }

  cap_ret_t method_kill(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3) {
    using namespace caprese::task;

    auto [killable, killable_error] = is_permitted(cap, permission::KILLABLE);
    if (killable_error || !killable) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    auto [tid, tid_error] = get_field(cap, field::TID);
    if (tid_error) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    task_t* task = lookup(std::bit_cast<tid_t>(static_cast<uint32_t>(tid)));
    if (task == nullptr) [[unlikely]] {
      return { .result = 0, .error = 1 };
    }

    kill(task);

    return { .result = 0, .error = 0 };
  }
} // namespace caprese::capability::bic::task
