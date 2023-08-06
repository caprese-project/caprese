/**
 * @file string.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief string.h
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstring>

extern "C" {
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

  void* memset(void* src, int ch, size_t n) {
    unsigned char  c = ch;
    unsigned char* p = reinterpret_cast<unsigned char*>(src);
    while (n-- > 0) {
      *p++ = c;
    }
    return src;
  }

  void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char*       cdest = reinterpret_cast<unsigned char*>(dest);
    const unsigned char* csrc  = reinterpret_cast<const unsigned char*>(src);

    for (size_t i = 0; i < n; i++) {
      cdest[i] = csrc[i];
    }

    return dest;
  }
}
