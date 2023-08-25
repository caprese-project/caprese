#ifndef RUNTIME_CAP_H_
#define RUNTIME_CAP_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define HANDLE_TYPE_NULL   0
#define HANDLE_TYPE_MEMORY 1
#define HANDLE_TYPE_TASK   2

  uint16_t get_handle_type(handle_t handle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_CAP_H_
