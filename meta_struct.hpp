#ifndef GKXX_META_STRUCT_HPP
#define GKXX_META_STRUCT_HPP

#include <algorithm>
#include <compare>
#include <concepts>
#include <string>
#include <string_view>

#include "is_specialization_of.hpp"

namespace gkxx {

template <std::size_t N>
struct string_literal {
  constexpr string_literal(const char (&str)[N + 1]) {
    std::copy_n(str, N + 1, data);
  }
  auto operator<=>(const string_literal &) const = default;
  constexpr std::string_view sv() const {
    return {data, N};
  }
  char data[N + 1]{};
};

template <std::size_t N>
string_literal(const char (&)[N]) -> string_literal<N - 1>;

template <string_literal Tag, typename T>
struct tag_value_pair {
  T &&value;
};

template <string_literal Tag>
struct arg_t {
  template <typename T>
  constexpr auto operator=(T &&value) const {
    return tag_value_pair<Tag, T>{std::forward<T>(value)};
  }
};

template <string_literal Tag>
inline constexpr arg_t<Tag> arg{};

template <typename T>
struct default_init {
  constexpr auto operator()() const {
    if constexpr (std::is_default_constructible_v<T>)
      return T{};
  }
};

struct required_t {
  explicit required_t() = default;

  template <typename T>
  explicit operator T() const noexcept; // not defined

  template <string_literal Tag>
  static inline constexpr auto required_arg_specified = false;
};

inline constexpr required_t required{};

template <typename... TVPs>
struct param_pack : TVPs... {};

template <typename... TVPs>
param_pack(TVPs &&...) -> param_pack<std::remove_reference_t<TVPs>...>;

template <string_literal Tag, typename T, auto Init = default_init<T>{}>
struct member {
  template <typename MS>
  constexpr member(MS &ms) : value(call_init(ms)) {}
  template <typename MS, typename... TVPs>
  constexpr member(MS &ms, param_pack<TVPs...> &&params)
      : value(try_init(ms, std::move(params))) {}
  static constexpr auto tag() noexcept {
    return Tag;
  }
  static constexpr auto init() noexcept {
    return Init;
  }
  using element_type = T;
  T value;

 private:
  static constexpr decltype(auto) call_init(auto &)
    requires requires {
      { Init() } -> std::convertible_to<T>;
    }
  {
    return Init();
  }
  static constexpr decltype(auto) call_init(auto &ms)
    requires requires {
      { Init(ms) } -> std::convertible_to<T>;
    }
  {
    return Init(ms);
  }
  static constexpr decltype(auto) call_init(auto &)
    requires std::same_as<std::remove_cvref_t<decltype(Init)>, required_t>
  {
    static_assert(required_t::required_arg_specified<Tag>,
                  "required argument not specified");
    return required_t{};
  }

  template <typename MS, typename OtherT>
  constexpr decltype(auto) try_init(MS &,
                                    tag_value_pair<Tag, OtherT> tvp) const {
    return std::forward<OtherT>(tvp.value);
  }
  template <typename MS>
  constexpr decltype(auto) try_init(MS &ms, ...) const {
    return call_init(ms);
  }
};

template <typename... Members>
struct meta_struct;

namespace detail {

  template <typename... Members>
  struct meta_struct_impl : Members... {
    template <typename... TVPs>
    constexpr meta_struct_impl(meta_struct<Members...> &ms,
                               param_pack<TVPs...> &&p)
        : Members(ms, std::move(p))... {}
    constexpr meta_struct_impl(meta_struct<Members...> &ms) : Members(ms)... {}
  };

} // namespace detail

template <typename... Members>
struct meta_struct : detail::meta_struct_impl<Members...> {
 private:
  using super = detail::meta_struct_impl<Members...>;

 public:
  template <typename... TVPs>
  constexpr meta_struct(TVPs &&...tvps)
      : super(*this, param_pack{std::move(tvps)...}) {}
  constexpr meta_struct() : super(*this) {}
};

namespace detail {

  template <string_literal Tag, typename T, auto Init>
  inline constexpr T &get_impl(member<Tag, T, Init> &m) {
    return m.value;
  }

  template <string_literal Tag, typename T, auto Init>
  inline constexpr T &&get_impl(member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

  template <string_literal Tag, typename T, auto Init>
  inline constexpr const T &get_impl(const member<Tag, T, Init> &m) {
    return m.value;
  }

  template <string_literal Tag, typename T, auto Init>
  inline constexpr const T &&get_impl(const member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

} // namespace detail

template <typename T>
concept CMetaStruct_cvr =
    specialization_of<std::remove_cvref_t<T>, meta_struct>;

template <string_literal Tag, CMetaStruct_cvr MS>
inline constexpr decltype(auto) get(MS &&ms) {
  return detail::get_impl<Tag>(std::forward<MS>(ms));
}

} // namespace gkxx

#endif // GKXX_META_STRUCT_HPP