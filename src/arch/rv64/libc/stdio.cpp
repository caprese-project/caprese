#include <cstdio>

#include <kernel/address.h>
#include <kernel/arch/sbi.h>

namespace {
  size_t stdio_read(void*, size_t, size_t, FILE*) {
    return 0;
  }

  size_t stdio_write(const void* __restrict ptr, size_t size, size_t nmemb, FILE*) {
    sbi_debug_console_write(size * nmemb, map_ptr<char>::from(ptr).as_phys().raw(), 0);
    return size * nmemb;
  }

  int stdio_ungetc(int, FILE*) {
    return EOF;
  }

  char stdio_buf[BUFSIZ];

  FILE stdio_file = {
    .__fd         = 0,
    .__mode       = _IOLBF,
    .__ungetc_buf = static_cast<char>(EOF),
    .__buf        = stdio_buf,
    .__buf_size   = BUFSIZ,
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
