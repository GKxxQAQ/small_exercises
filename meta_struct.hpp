#ifndef GKXX_META_STRUCT_HPP
#define GKXX_META_STRUCT_HPP

#include <algorithm>
#include <compare>
#include <concepts>
#include <string>

namespace gkxx {

template <std::size_t N>
struct string_literal {
  constexpr string_literal(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, data);
  }
  auto operator<=>(const string_literal &) const = default;
  char data[N + 1]{};
};

template <std::size_t N>
string_literal(const char (&)[N]) -> string_literal<N - 1>;

template <string_literal Tag, typename T>
struct member {
  static constexpr auto tag() noexcept {
    return Tag;
  }
  using element_type = T;
  T value;
};

namespace detail {

  template <typename T>
  inline constexpr auto is_member = false;
  template <string_literal Tag, typename T>
  inline constexpr auto is_member<member<Tag, T>> = true;

} // namespace detail

template <typename T>
concept CMember = detail::is_member<T>;

template <CMember... Members>
struct meta_struct : Members... {};

namespace detail {

  template <typename T>
  inline constexpr auto is_meta_struct = false;
  template <CMember... Members>
  inline constexpr auto is_meta_struct<meta_struct<Members...>> = true;

  template <string_literal Tag, typename T>
  inline constexpr decltype(auto) get_impl(member<Tag, T> &m) {
    return (m.value);
  }

} // namespace detail

template <typename T>
concept CMetaStruct = detail::is_meta_struct<std::remove_cvref_t<T>>;

template <string_literal Tag, CMetaStruct MS>
inline constexpr decltype(auto) get(MS &&ms) {
  return detail::get_impl<Tag>(std::forward<MS>(ms));
}

namespace detail {

  template <typename Tag, typename T>
  struct tag_value_pair {
    T &&value;
  };

} // namespace detail

template <string_literal Tag>
struct arg_type {
  template <typename T>
  constexpr auto operator=(T &&value) const {
    return tag_value_pair<Tag, T>{std::forward<T>(value)};
  }
};

template <string_literal Tag>
inline constexpr arg_type<Tag> arg{};

} // namespace gkxx

#endif // GKXX_META_STRUCT_HPP