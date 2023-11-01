#include <kernel/sbi.h>

extern "C" {
  struct __FILE {
    void write(int ch) {
      sbi_console_putchar(ch);
    }
  };

  namespace {
    struct __FILE __stdout_file;
  } // namespace

  struct __FILE* __stdout() {
    return &__stdout_file;
  }

  int fputc(int ch, struct __FILE* stream) {
    stream->write(ch);
    return ch;
  }
}
