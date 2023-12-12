#include <cstdio>

#include <kernel/log.h>

void log(log_level_t level, const char* tag, const char* fmt, va_list ap) {
  const char* level_str = "";

  switch (level) {
    case log_level_t::DEBUG:
      level_str = TERM_COLOR_CYAN "[DEBUG] " TERM_RESET;
      break;
    case log_level_t::INFO:
      level_str = TERM_COLOR_GREEN "[INFO]  " TERM_RESET;
      break;
    case log_level_t::WARNING:
      level_str = TERM_COLOR_YELLOW "[WARN]  " TERM_RESET;
      break;
    case log_level_t::ERROR:
      level_str = TERM_COLOR_RED "[ERROR] " TERM_RESET;
      break;
    case log_level_t::FATAL:
      level_str = TERM_COLOR_RED "[" TERM_STYLE_BOLD "FATAL" TERM_RESET TERM_COLOR_RED "] " TERM_RESET;
      break;
  }

  printf("%s" TERM_COLOR_MAGENTA "%s" TERM_RESET ": ", level_str, tag);
  vprintf(fmt, ap);
  lognl();
}

void lognl() {
  if constexpr (CONFIG_LOG_DEBUG || CONFIG_LOG_INFO || CONFIG_LOG_WARNING || CONFIG_LOG_ERROR || CONFIG_LOG_FATAL) {
    putchar('\n');
  }
}
