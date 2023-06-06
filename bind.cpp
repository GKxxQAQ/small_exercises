#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace placeholders {

template <unsigned N>
struct placeholder {};

template <typename>
struct is_placeholder {
  static constexpr auto value = 0u;
};

template <unsigned N>
struct is_placeholder<placeholder<N>> {
  static constexpr auto value = N;
};

namespace detail {

  template <unsigned N>
  struct tens {
    static constexpr unsigned value = 10 * tens<N - 1>::value;
  };

  template <>
  struct tens<0u> {
    static constexpr unsigned value = 1;
  };

  template <char...>
  struct decimal_value;

  template <>
  struct decimal_value<> {
    static constexpr unsigned value = 0;
  };

  template <char first, char... rest>
  struct decimal_value<first, rest...> {
    static constexpr unsigned value =
        (first - '0') * tens<sizeof...(rest)>::value +
        decimal_value<rest...>::value;
  };

} // namespace detail

template <char... digits>
inline constexpr placeholder<detail::decimal_value<digits...>::value>
operator""_ph() {
  return {};
}

} // namespace placeholders

template <typename Func, typename... BoundArgs>
struct bind_result {
 private:
  Func functor;
  std::tuple<BoundArgs...> bound_args;

  using helper_sequence = std::index_sequence_for<BoundArgs...>;

 public:
  template <typename F, typename... BA>
  bind_result(F &&f, BA &&...ba) : functor(f), bound_args(ba...) {}

 private:
  template <typename>
  struct is_reference_wrapper : std::false_type {};

  template <typename T>
  struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

  template <typename>
  struct helper {};

  template <std::size_t... indices>
  struct helper<std::integer_sequence<std::size_t, indices...>> {
    Func &functor;
    std::tuple<BoundArgs...> &bound_args;

    template <std::size_t I, typename BoundT, typename... Args>
    decltype(auto) take_argument(Args &&...args) {
      constexpr auto ph_value =
          placeholders::is_placeholder<std::remove_cv_t<BoundT>>::value;
      if constexpr (ph_value)
        return std::get<ph_value - 1>(
            std::forward_as_tuple(std::forward<Args>(args)...));
      else
        return std::get<I>(bound_args);
    }

    template <typename... Args>
    decltype(auto) call_with(Args &&...args) {
      return functor(
          take_argument<indices, BoundArgs>(std::forward<Args>(args)...)...);
    }
  };

 public:
  template <typename... Args>
  decltype(auto) operator()(Args &&...args) {
    return helper<helper_sequence>{functor, bound_args}.call_with(
        std::forward<Args>(args)...);
  }
};

template <typename Func, typename... BoundArgs>
bind_result<std::decay_t<Func>, std::decay_t<BoundArgs>...>
bind(Func &&functor, BoundArgs &&...boundArgs) {
  return {std::forward<Func>(functor), std::forward<BoundArgs>(boundArgs)...};
}

void f(int n1, int n2, int n3, const int &n4, int n5) {
  std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

struct X {
  X(const X &) {
    std::cout << "X is copied" << std::endl;
  }
  X &operator=(const X &) {
    std::cout << "X is copied (=)" << std::endl;
    return *this;
  }
  X(X &&) noexcept {
    std::cout << "X is moved" << std::endl;
  }
  X &operator=(X &&) noexcept {
    std::cout << "X is moved (=)" << std::endl;
    return *this;
  }
  X() = default;
};

void foo(X &) {}

int main() {
  using namespace placeholders;
  int n = 7;
  auto f1 = bind(f, 2_ph, 42, 1_ph, std::cref(n), n);
  n = 10;
  f1(1, 2, 1001);
  X x{};
  auto f2 = bind(foo, x);
  f2();
}