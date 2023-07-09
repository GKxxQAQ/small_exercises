#ifndef GKXX_STRING_LITERAL_HPP
#define GKXX_STRING_LITERAL_HPP

#include <compare>
#include <string_view>

namespace gkxx {

/// @brief Compile-time fixed string, enabling the use of string literals as
/// template parameters and user-literals
/// @tparam N The length of the string
template <std::size_t N>
struct fixed_string {
  constexpr fixed_string(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, data);
  }
  auto operator<=>(const fixed_string &) const = default;
  bool operator==(const fixed_string &) const = default;
  constexpr std::string_view as_sv() const noexcept {
    return {data, N};
  }
  static constexpr auto size() noexcept {
    return N;
  }
  char data[N + 1]{};
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N - 1>;

template <std::size_t N, std::size_t M>
  requires(N != M)
inline constexpr bool operator==(const fixed_string<N> &,
                                 const fixed_string<M> &) {
  return false;
}

} // namespace gkxx

#endif // GKXX_STRING_LITERAL_HPP