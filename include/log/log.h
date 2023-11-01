#ifndef LOG_LOG_H_
#define LOG_LOG_H_

#include <cstdarg>
#include <cstdlib>

#ifdef CONFIG_ANSI_ESC_SEQ
#define TERM_COLOR_BLACK   "\e[30m"
#define TERM_COLOR_RED     "\e[31m"
#define TERM_COLOR_GREEN   "\e[32m"
#define TERM_COLOR_YELLOW  "\e[33m"
#define TERM_COLOR_BLUE    "\e[34m"
#define TERM_COLOR_MAGENTA "\e[35m"
#define TERM_COLOR_CYAN    "\e[36m"
#define TERM_COLOR_WHITE   "\e[37m"

#define TERM_STYLE_BOLD      "\e[1m"
#define TERM_STYLE_ITALIC    "\e[3m"
#define TERM_STYLE_UNDERLINE "\e[4m"

#define TERM_RESET "\e[0m"
#else
#define TERM_COLOR_BLACK   ""
#define TERM_COLOR_RED     ""
#define TERM_COLOR_GREEN   ""
#define TERM_COLOR_YELLOW  ""
#define TERM_COLOR_BLUE    ""
#define TERM_COLOR_MAGENTA ""
#define TERM_COLOR_CYAN    ""
#define TERM_COLOR_WHITE   ""

#define TERM_STYLE_BOLD      ""
#define TERM_STYLE_ITALIC    ""
#define TERM_STYLE_UNDERLINE ""

#define TERM_RESET ""
#endif

enum struct log_level_t {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL,
};

void log(log_level_t level, const char* tag, const char* fmt, va_list ap);
void lognl();

inline void logd(const char* tag, const char* fmt, ...) {
  if constexpr (CONFIG_LOG_DEBUG) {
    va_list ap;
    va_start(ap, fmt);
    log(log_level_t::DEBUG, tag, fmt, ap);
    va_end(ap);
  }
}

inline void logi(const char* tag, const char* fmt, ...) {
  if constexpr (CONFIG_LOG_INFO) {
    va_list ap;
    va_start(ap, fmt);
    log(log_level_t::INFO, tag, fmt, ap);
    va_end(ap);
  }
}

inline void logw(const char* tag, const char* fmt, ...) {
  if constexpr (CONFIG_LOG_WARNING) {
    va_list ap;
    va_start(ap, fmt);
    log(log_level_t::WARNING, tag, fmt, ap);
    va_end(ap);
  }
}

inline void loge(const char* tag, const char* fmt, ...) {
  if constexpr (CONFIG_LOG_ERROR) {
    va_list ap;
    va_start(ap, fmt);
    log(log_level_t::ERROR, tag, fmt, ap);
    va_end(ap);
  }
}

inline void logf(const char* tag, const char* fmt, ...) {
  if constexpr (CONFIG_LOG_FATAL) {
    va_list ap;
    va_start(ap, fmt);
    log(log_level_t::FATAL, tag, fmt, ap);
    va_end(ap);
  }
}

#ifdef NDEBUG
#define panic(fmt, ...)                        \
do {                                           \
if constexpr (CONFIG_LOG_FATAL) {              \
logf("panic", fmt __VA_OPT__(, ) __VA_ARGS__); \
dump();                                        \
}                                              \
abort();                                       \
} while (false)
#else // ^^^ NDEBUG
#define panic(fmt, ...)                                                                     \
do {                                                                                        \
if constexpr (CONFIG_LOG_FATAL) {                                                           \
logf("panic", fmt " at %s:%d %s" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__, __func__); \
dump();                                                                                     \
}                                                                                           \
abort();                                                                                    \
} while (false)
#endif // !NDEBUG

#define unimpl() panic("An unimplemented code has been reached.")

#endif // LOG_LOG_H_
