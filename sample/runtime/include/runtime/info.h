#ifndef RUNTIME_INFO_H_
#define RUNTIME_INFO_H_

#include <runtime/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  size_t    page_size();
  uintptr_t user_space_start();
  uintptr_t user_space_end();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_INFO_H_
