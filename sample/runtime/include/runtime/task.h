#ifndef RUNTIME_TASK_H_
#define RUNTIME_TASK_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define TASK_STATE_UNUSED   0
#define TASK_STATE_CREATING 1
#define TASK_STATE_RUNNING  2
#define TASK_STATE_READY    3

  tid_t         get_tid(void);
  task_handle_t create_task(void);
  void          yield(void);
  task_state_t  task_state(task_handle_t task);
  bool          kill_task(task_handle_t task, uintptr_t exit_code);
  void          switch_task(task_handle_t task);
  bool          set_register(task_handle_t task, uintptr_t reg, uintptr_t value);
  bool          ipc_send(task_handle_t task, const ipc_msg_t* msg);
  bool          ipc_receive(ipc_msg_t* msg);

  task_handle_t this_task_handle(void);

  task_handle_t lookup_task(const char* name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_TASK_H_
