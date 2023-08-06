#ifndef CAPRESE_UTIL_LAMBDA_H_
#define CAPRESE_UTIL_LAMBDA_H_

#include <tuple>
#include <utility>

namespace caprese::inline util {
  template<typename F>
  class recursive_lambda: private F {
  public:
    template<typename G>
    explicit constexpr recursive_lambda(G&& g): F { std::forward<G>(g) } { }

    template<typename... Args>
    constexpr decltype(auto) operator()(Args&&... args) const {
      return F::operator()(*this, std::forward<Args>(args)...);
    }
  };

  template<typename F>
  constexpr inline decltype(auto) make_recursive_lambda(F&& f) {
    return recursive_lambda<std::decay_t<F>>(std::forward<std::decay_t<F>>(f));
  }

  template<size_t N = 0, typename T, typename F>
  void iterate_tuple(const T& t, F callback) {
    if constexpr (N < std::tuple_size<T>::value) {
      callback(std::get<N>(t));
      iterate_tuple<N + 1>(t, callback);
    }
  }
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_LAMBDA_H_
