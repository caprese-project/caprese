#include <cstdio>

#include <kernel/arch/sbi.h>

namespace {
  size_t stdio_read(void*, size_t, size_t, FILE*) {
    return 0;
  }

  size_t stdio_write(const void* __restrict ptr, size_t size, size_t nmemb, FILE*) {
    for (size_t i = 0; i < size * nmemb; ++i) {
      sbi_console_putchar(static_cast<const char*>(ptr)[i]);
    }
    return size * nmemb;
  }

  int stdio_ungetc(int, FILE*) {
    return EOF;
  }

  FILE stdio_file = {
    .__buf_mode   = _IONBF,
    .__ungetc_buf = static_cast<char>(EOF),
    .__buf        = nullptr,
    .__buf_size   = 0,
    .__buf_pos    = 0,
    .__read       = stdio_read,
    .__write      = stdio_write,
    .__ungetc     = stdio_ungetc,
  };
} // namespace

extern "C" {
  FILE* __stdin() {
    return &stdio_file;
  }

  FILE* __stdout() {
    return &stdio_file;
  }

  FILE* __stderr() {
    return &stdio_file;
  }
}
