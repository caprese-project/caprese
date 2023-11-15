#include <cerrno>
#include <csignal>

#include <log/log.h>

namespace {
  __signal_handler_t __signal_handlers[NSIG];
} // namespace

extern "C" {
  int __raise(int sig) {
    if (sig < 0 || sig >= NSIG) {
      errno = EINVAL;
      return -1;
    }

    __signal_handler_t handler = __signal_handlers[sig];
    if (handler == SIG_DFL) {
      panic("Unhandled signal.");
    } else if (handler == SIG_IGN) {
      // Do nothing.
    } else if (handler == SIG_ERR) {
      errno = EINVAL;
      return -1;
    } else {
      __signal_handlers[sig] = SIG_DFL;
      handler(sig);
    }

    return 0;
  }

  __signal_handler_t __signal(int sig, __signal_handler_t handler) {
    if (sig < 0 || sig >= NSIG) {
      errno = EINVAL;
      return SIG_ERR;
    }

    __signal_handler_t old_handler = __signal_handlers[sig];
    __signal_handlers[sig]         = handler;
    return old_handler;
  }
}
