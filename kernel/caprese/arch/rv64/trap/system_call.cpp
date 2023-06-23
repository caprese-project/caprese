/**
 * @file system_call.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstdio>

#include <caprese/arch/rv64/sbi.h>
#include <caprese/arch/rv64/task/task.h>
#include <caprese/arch/rv64/trap/system_call.h>
#include <caprese/task/task.h>

#include <libcaprese/system_call/system_call_type.h>

namespace caprese::arch::trap {
  namespace {
    using system_call_handler_t = void (*)(caprese::task::task_t*);

    void null_system_call([[maybe_unused]] caprese::task::task_t*) { }

    void debug_console_put_char(caprese::task::task_t* task) {
      sbi_console_putchar(task->arch_task.trap_frame.a0);
    }

    void create_thread_control_block([[maybe_unused]] caprese::task::task_t*) {
      printf("Unimplemented system call.\n");
    }

    void switch_thread_control_block([[maybe_unused]] caprese::task::task_t*) {
      printf("Unimplemented system call.\n");
    }

    void destroy_thread_control_block([[maybe_unused]] caprese::task::task_t*) {
      printf("Unimplemented system call.\n");
    }

    void get_thread_control_block_id(caprese::task::task_t* task) {
      task->arch_task.trap_frame.a0 = task->task_id.task_space_id;
    }

    constexpr system_call_handler_t system_calls[] = {
      [SYSTEM_CALL_NULL_SYSTEM_CALL]             = null_system_call,
      [SYSTEM_CALL_DEBUG_CONSOLE_PUT_CHAR]       = debug_console_put_char,
      [SYSTEM_CALL_CREATE_THREAD_CONTROL_BLOCK]  = create_thread_control_block,
      [SYSTEM_CALL_SWITCH_THREAD_CONTROL_BLOCK]  = switch_thread_control_block,
      [SYSTEM_CALL_DESTROY_THREAD_CONTROL_BLOCK] = destroy_thread_control_block,
      [SYSTEM_CALL_GET_THREAD_CONTROL_BLOCK_ID]  = get_thread_control_block_id,
    };
  } // namespace

  void handle_system_call() {
    auto task = caprese::task::get_current_task();
    if (task->arch_task.trap_frame.a7 < sizeof(system_calls) / sizeof(system_calls[0])) {
      system_calls[task->arch_task.trap_frame.a7](task);
    } else {
      printf("Unknown system call: %p\n", reinterpret_cast<void*>(task->arch_task.trap_frame.a7));
    }
  }
} // namespace caprese::arch::trap
