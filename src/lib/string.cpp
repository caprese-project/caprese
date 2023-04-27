#include <caprese/lib/string.h>

namespace caprese {
  int strcmp(const char* str1, const char* str2) {
    while (*str1 != '\0' && *str1 == *str2) {
      ++str1;
      ++str2;
    }
    return *str1 - *str2;
  }

  int strncmp(const char* str1, const char* str2, size_t length) {
    int res = 0;
    while (length) {
      res = *str1 - *str2;
      ++str1;
      ++str2;
      if (res != 0 || *str1 == '\0' || *str2 == '\0') {
        break;
      }
      --length;
    }
    return res;
  }

  size_t strlen(const char* str) {
    size_t length = 0;
    while (str[length]) {
      ++length;
    }
    return length;
  }

  char* strchr(const char* str, int ch) {
    char* p = const_cast<char*>(str);
    while (true) {
      if (*p == ch) {
        return p;
      }
      if (*p == '\0') {
        return nullptr;
      }
      ++p;
    }
  }
} // namespace caprese
