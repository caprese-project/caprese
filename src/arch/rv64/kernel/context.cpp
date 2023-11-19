#include <kernel/context.h>

extern "C" {
  extern void              _switch_context(context_t*, context_t*);
  [[noreturn]] extern void _load_context(context_t*);
}

void switch_context(map_ptr<context_t> new_context, map_ptr<context_t> old_context) {
  _switch_context(new_context.get(), old_context.get());
}

[[noreturn]] void load_context(map_ptr<context_t> context) {
  _load_context(context.get());
}
