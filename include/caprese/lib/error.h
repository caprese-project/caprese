#ifndef CAPRESE_LIB_ERROR_H_
#define CAPRESE_LIB_ERROR_H_

namespace caprese {
  enum struct error_t {
    ok               = 0,
    invalid_argument = -1,
    not_found        = -2,
  };

  inline bool succeeded(error_t err) {
    return static_cast<int>(err) >= 0;
  }

  inline bool failed(error_t err) {
    return static_cast<int>(err) < 0;
  }
} // namespace caprese

#endif // CAPRESE_LIB_ERROR_H_
