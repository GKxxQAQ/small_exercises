#ifndef GKXX_META_STRUCT_HPP
#define GKXX_META_STRUCT_HPP

#include <algorithm>
#include <compare>
#include <concepts>
#include <string>

#include "is_specialization_of.hpp"

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
struct tag_value_pair {
  T &&value;
};

namespace detail {

  template <typename T>
  inline constexpr auto is_tag_value_pair = false;
  template <string_literal Tag, typename T>
  inline constexpr auto is_tag_value_pair<tag_value_pair<Tag, T>> = true;

} // namespace detail

template <typename T>
concept CTagValuePair_cvr = detail::is_tag_value_pair<std::remove_cvref_t<T>>;

template <string_literal Tag>
struct arg_type {
  template <typename T>
  constexpr auto operator=(T &&value) const {
    return tag_value_pair<Tag, T>{std::forward<T>(value)};
  }
};

template <string_literal Tag>
inline constexpr arg_type<Tag> arg{};

template <typename T>
struct default_init {
  constexpr auto operator()() const {
    if constexpr (std::is_default_constructible_v<T>)
      return T{};
  }
};

template <string_literal Tag, typename T, auto Init = default_init<T>{}>
struct member {
  template <typename OtherT>
  constexpr member(tag_value_pair<Tag, OtherT> tvp)
      : value(std::forward<OtherT>(tvp.value)) {}
  template <typename MS>
  constexpr member(MS &ms) : value(call_init(ms)) {}
  static constexpr auto tag() noexcept {
    return Tag;
  }
  static constexpr auto init() noexcept {
    return Init;
  }
  using element_type = T;
  T value;

 private:
  template <typename MS>
  static constexpr decltype(auto) call_init(MS &)
    requires requires {
      { Init() } -> std::convertible_to<T>;
    }
  {
    return Init();
  }
  template <typename MS>
  static constexpr decltype(auto) call_init(MS &ms)
    requires requires {
      { Init(ms) } -> std::convertible_to<T>;
    }
  {
    return Init(ms);
  }
};

namespace detail {

  template <typename T>
  inline constexpr auto is_member = false;
  template <string_literal Tag, typename T, auto Init>
  inline constexpr auto is_member<member<Tag, T, Init>> = true;

} // namespace detail

template <typename T>
concept CMember = detail::is_member<T>;

template <CTagValuePair_cvr... TVPs>
struct param_pack : TVPs... {};

template <CTagValuePair_cvr... TVPs>
param_pack(TVPs &&...) -> param_pack<TVPs...>;

template <typename T>
concept CParamPack_cvr = specialization_of<std::remove_cvref_t<T>, param_pack>;

template <CMember... Members>
struct meta_struct;

namespace detail {

  template <CMember... Members>
  struct meta_struct_impl : Members... {
    template <CParamPack_cvr Params>
    constexpr meta_struct_impl(Params &&p) : Members(std::move(p))... {}
    constexpr meta_struct_impl(meta_struct<Members...> &ms) : Members(ms)... {}
  };

} // namespace detail

template <CMember... Members>
struct meta_struct : detail::meta_struct_impl<Members...> {
 private:
  using super = detail::meta_struct_impl<Members...>;

 public:
  template <CTagValuePair_cvr... TVPs>
  constexpr meta_struct(TVPs &&...tvps)
      : super(param_pack{std::move(tvps)...}) {}
  constexpr meta_struct() : super(*this) {}
};

namespace detail {

  template <string_literal Tag, typename T, auto Init>
  inline constexpr decltype(auto) get_impl(member<Tag, T, Init> &m) {
    return (m.value);
  }

} // namespace detail

template <typename T>
concept CMetaStruct_cvr =
    specialization_of<std::remove_cvref_t<T>, meta_struct>;

// TODO: get for rvalue and cv-qualification
template <string_literal Tag, CMetaStruct_cvr MS>
inline constexpr decltype(auto) get(MS &&ms) {
  return detail::get_impl<Tag>(std::forward<MS>(ms));
}

} // namespace gkxx

#endif // GKXX_META_STRUCT_HPP