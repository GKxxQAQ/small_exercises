#ifndef GKXX_STRING_LITERAL_HPP
#define GKXX_STRING_LITERAL_HPP

#include <string_view>
#include <compare>

namespace gkxx {

/// @brief Compile-time string literal, enabling the use of string literals as
/// template parameters and user-literals
/// @tparam N The length of the string
template <std::size_t N>
struct string_literal {
  constexpr string_literal(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, data);
  }
  auto operator<=>(const string_literal &) const = default;
  bool operator==(const string_literal &) const = default;
  constexpr std::string_view as_sv() const {
    return {data, N};
  }
  char data[N + 1]{};
};

template <std::size_t N>
string_literal(const char (&)[N]) -> string_literal<N - 1>;

template <std::size_t N, std::size_t M>
  requires(N != M)
inline constexpr bool operator==(const string_literal<N> &,
                                 const string_literal<M> &) {
  return false;
}

} // namespace gkxx

#endif // GKXX_STRING_LITERAL_HPP