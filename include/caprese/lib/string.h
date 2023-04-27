#ifndef CAPRESE_LIB_STRING_H_
#define CAPRESE_LIB_STRING_H_

#include <cstddef>

namespace caprese {
  int    strcmp(const char* str1, const char* str2);
  int    strncmp(const char* str1, const char* str2, size_t length);
  size_t strlen(const char* str);
  char*  strchr(const char* str, int ch);
} // namespace caprese

#endif // CAPRESE_LIB_STRING_H_
