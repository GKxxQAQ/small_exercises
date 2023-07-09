#ifndef GKXX_META_STRUCT_HPP
#define GKXX_META_STRUCT_HPP

#include <algorithm>
#include <compare>
#include <concepts>
#include <string>
#include <string_view>

#include "fixed_string.hpp"
#include "is_specialization_of.hpp"

namespace gkxx::meta_struct {

template <fixed_string Tag, typename T>
struct tag_value_pair {
  T &&value;
};

template <fixed_string Tag>
struct initializer_t {
  template <typename T>
  constexpr auto operator=(T &&value) const {
    return tag_value_pair<Tag, T>{std::forward<T>(value)};
  }
};

template <fixed_string Tag>
inline constexpr initializer_t<Tag> init{};

template <fixed_string Tag>
inline constexpr initializer_t<Tag> operator""_init() {
  return {};
}

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
  explicit operator T() const; // not defined

  template <fixed_string Tag>
  static inline constexpr auto required_arg_specified = false;
};

inline constexpr required_t required{};

template <typename... TVPs>
struct param_pack : TVPs... {};

template <typename... TVPs>
param_pack(TVPs &&...) -> param_pack<std::remove_reference_t<TVPs>...>;

template <fixed_string Tag, typename T, auto Init = default_init<T>{}>
struct member {
  constexpr member(auto &ms) : value(call_init(ms)) {}
  template <typename... TVPs>
  constexpr member(auto &ms, param_pack<TVPs...> &&params)
      : value(try_init(ms, std::move(params))) {}
  static constexpr auto tag() {
    return Tag;
  }
  static constexpr auto init() {
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

  template <typename OtherT>
  static constexpr decltype(auto) try_init(auto &,
                                           tag_value_pair<Tag, OtherT> tvp) {
    return std::forward<OtherT>(tvp.value);
  }
  static constexpr decltype(auto) try_init(auto &ms, ...) {
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

template <fixed_string... Tags>
struct tag_list {
  template <template <typename...> typename Container>
  static constexpr Container<std::string_view> as_container() {
    return {Tags.as_sv()...};
  }
  template <fixed_string S>
  static inline constexpr auto contains = (... || (Tags == S));
};

template <typename... Types>
struct type_list {
  template <typename Func>
    requires(... && std::invocable<Func, Types *>)
  static constexpr void apply(Func &&func) {
    [[maybe_unused]] int _[] = {
        (std::forward<Func>(func)(static_cast<Types *>(nullptr)), 0)...};
  }
  template <typename T>
  static inline constexpr auto contains = (... || std::is_same_v<Types, T>);
};

template <typename... Members>
struct member_list : type_list<Members...> {
  using type_list<Members...>::apply;
  using tags = tag_list<Members::tag()...>;
  using element_types = type_list<typename Members::element_type...>;

  template <fixed_string S>
  static inline constexpr auto contains_tag = tags::template contains<S>;

  template <typename T>
  static inline constexpr auto contains_element =
      element_types::template contains<T>;
};

template <typename... Members>
struct meta_struct : detail::meta_struct_impl<Members...> {
 private:
  using super = detail::meta_struct_impl<Members...>;

 public:
  constexpr meta_struct(auto &&...tvpairs)
      : super(*this, param_pack{std::move(tvpairs)...}) {}
  constexpr meta_struct() : super(*this) {}

  using members = member_list<Members...>;
};

namespace detail {

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr T &get_impl(member<Tag, T, Init> &m) {
    return m.value;
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr T &&get_impl(member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr const T &get_impl(const member<Tag, T, Init> &m) {
    return m.value;
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr const T &&get_impl(const member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr volatile T &get_impl(volatile member<Tag, T, Init> &m) {
    return m.value;
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr volatile T &&get_impl(volatile member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr const volatile T &
  get_impl(const volatile member<Tag, T, Init> &m) {
    return m.value;
  }

  template <fixed_string Tag, typename T, auto Init>
  inline constexpr const volatile T &&
  get_impl(const volatile member<Tag, T, Init> &&m) {
    return std::move(m.value);
  }

} // namespace detail

template <fixed_string Tag, typename MS>
  requires specialization_of<std::remove_cvref_t<MS>, meta_struct>
inline constexpr decltype(auto) get(MS &&ms) {
  return detail::get_impl<Tag>(std::forward<MS>(ms));
}

} // namespace gkxx::meta_struct

#endif // GKXX_META_STRUCT_HPP