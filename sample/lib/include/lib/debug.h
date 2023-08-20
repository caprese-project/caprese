#ifndef LIB_DEBUG_H_
#define LIB_DEBUG_H_

#ifndef NDEBUG
void _printd(const char* fmt, ...);
#define printd(fmt, ...) _printd(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define printd(fmt, ...) ((void)0)
#endif // NDEBUG

#endif // LIB_DEBUG_H_
