template <unsigned L, unsigned R, unsigned N>
struct any_divides {
  static constexpr auto result =
      (N % L == 0) || any_divides<L + 1, R, N>::result;
};
template <unsigned L, unsigned N>
struct any_divides<L, L, N> {
  static constexpr auto result = (N % L == 0);
};

template <unsigned N>
struct is_prime {
  static constexpr auto result = !any_divides<2, N - 1, N>::result;
};
template <>
struct is_prime<1u> {
  static constexpr auto result = false;
};
template <>
struct is_prime<2u> {
  static constexpr auto result = true;
};

template <bool...>
struct bool_list {};

template <bool, typename>
struct head_and_tail;

template <bool head, bool... tail>
struct head_and_tail<head, bool_list<tail...>> {
  using type = bool_list<head, tail...>;
};

template <template <unsigned> typename F, unsigned L, unsigned R>
struct result_sequence_impl {
  using type = typename head_and_tail<
      F<L>::result, typename result_sequence_impl<F, L + 1, R>::type>::type;
};

template <template <unsigned> typename F, unsigned L>
struct result_sequence_impl<F, L, L> {
  using type = bool_list<F<L>::result>;
};

template <template <unsigned> typename F, unsigned L, unsigned R>
using result_sequence = typename result_sequence_impl<F, L, R>::type;

template <unsigned... vals>
struct set {
  template <unsigned x>
  struct contains {
    static constexpr auto result = (... || (x == vals));
  };
};

template <unsigned L, unsigned R, unsigned... true_vals>
using bool_sequence =
    result_sequence<set<true_vals...>::template contains, L, R>;

template <typename X, typename Y>
inline constexpr auto same_type = false;

template <typename X>
inline constexpr auto same_type<X, X> = true;

int main() {
  using results = result_sequence<is_prime, 1, 100>;
  using answers =
      bool_sequence<1, 100, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43,
                    47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97>;
  static_assert(same_type<results, answers>);
}