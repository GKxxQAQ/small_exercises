#ifndef GKXX_UNIQUE_PTR_HPP
#define GKXX_UNIQUE_PTR_HPP

#include "default_delete.hpp"

#include <compare>
#include <concepts>
#include <functional>
#include <iostream>
#include <utility>

#ifndef CXX23_CONSTEXPR
#if __cplusplus > 202002L
#define CXX23_CONSTEXPR constexpr
#else
#define CXX23_CONSTEXPR
#endif
#endif

namespace gkxx {

namespace detail {

  template <typename T, typename D>
  auto pointer_helper(int) -> typename std::remove_reference_t<D>::pointer;
  template <typename T, typename D>
  auto pointer_helper(...) -> T *;

  template <typename Deleter>
  concept deleter_constraint =
      std::is_default_constructible_v<Deleter> && !std::is_pointer_v<Deleter>;

} // namespace detail

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
  static_assert(!std::is_rvalue_reference_v<Deleter>,
                "unique_ptr's deleter type must be a function object type"
                " or an lvalue reference type");

  static inline constexpr auto deleter_is_lref =
      std::is_lvalue_reference_v<Deleter>;
  using unref_deleter = std::remove_reference_t<Deleter>;

 public:
  using pointer = decltype(detail::pointer_helper<T, Deleter>(0));
  using element_type = T;
  using deleter_type = Deleter;

 private:
  pointer m_ptr{};
  [[no_unique_address]] Deleter m_deleter{};

 public:
  // (1)
  constexpr unique_ptr() noexcept
    requires(detail::deleter_constraint<Deleter>)
  = default;

  constexpr unique_ptr(std::nullptr_t) noexcept
    requires(detail::deleter_constraint<Deleter>)
      : unique_ptr() {}

  // (2)
  CXX23_CONSTEXPR explicit unique_ptr(pointer p) noexcept
    requires(detail::deleter_constraint<Deleter>)
      : m_ptr(p) {}

  // (3) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(pointer p, const Deleter &d) noexcept
    requires(!deleter_is_lref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(pointer p, Deleter &&d) noexcept
    requires(!deleter_is_lref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(std::move(d)) {}

  // (3) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(pointer p, Deleter d) noexcept
    requires(deleter_is_lref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(pointer p, unref_deleter &&d)
    requires(deleter_is_lref && std::is_constructible_v<Deleter, decltype(d)>)
  = delete;

  // (5) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(!deleter_is_lref && std::is_move_constructible_v<Deleter>)
      : m_ptr(other.release()), m_deleter(std::move(other.get_deleter())) {}

  // (5) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(deleter_is_lref)
      : m_ptr(other.release()), m_deleter(other.get_deleter()) {}

  // (6)
  template <typename U, typename E>
  CXX23_CONSTEXPR unique_ptr(unique_ptr<U, E> &&other) noexcept
    requires(std::convertible_to<typename unique_ptr<U, E>::pointer, pointer> &&
             !std::is_array_v<U> &&
             ((deleter_is_lref && std::is_same_v<Deleter, E>) ||
              !deleter_is_lref && std::convertible_to<E, Deleter>))
      : m_ptr(other.release()), m_deleter(std::forward<E>(other.get_deleter())) {}

  CXX23_CONSTEXPR ~unique_ptr() {
    get_deleter()(get());
  }

 private:
  template <typename U, typename E>
  CXX23_CONSTEXPR void do_assign(unique_ptr<U, E> &&other) noexcept {
    reset(other.release());
    get_deleter() = std::forward<E>(other.get_deleter());
  }

 public:
  // (1) move assignment operator
  CXX23_CONSTEXPR unique_ptr &operator=(unique_ptr &&other) noexcept
    requires(std::is_move_assignable_v<Deleter>)
  {
    if (this != &other)
      do_assign(std::move(other));
    return *this;
  }

  // (2) converting assignment operator
  template <typename U, typename E>
  CXX23_CONSTEXPR unique_ptr &operator=(unique_ptr<U, E> &&other) noexcept
    requires(!std::is_array_v<U> &&
             std::convertible_to<typename unique_ptr<U, E>::pointer, pointer> &&
             std::is_assignable_v<Deleter &, E &&>)
  {
    do_assign(std::move(other));
    return *this;
  }

  // (3)
  CXX23_CONSTEXPR unique_ptr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  CXX23_CONSTEXPR pointer release() noexcept {
    return std::exchange(m_ptr, nullptr);
  }

  CXX23_CONSTEXPR void reset(pointer p = pointer()) noexcept {
    auto old_ptr = std::exchange(m_ptr, p);
    if (old_ptr)
      get_deleter()(old_ptr);
  }

  void swap(unique_ptr &other) noexcept {
    m_data.swap(other.m_data);
  }

  CXX23_CONSTEXPR pointer get() const noexcept {
    return m_ptr;
  }

  CXX23_CONSTEXPR Deleter &get_deleter() noexcept {
    return m_deleter;
  }
  CXX23_CONSTEXPR const Deleter &get_deleter() const noexcept {
    return m_deleter;
  }

  CXX23_CONSTEXPR explicit operator bool() const noexcept {
    return get() != nullptr;
  }

  CXX23_CONSTEXPR auto operator*() const noexcept(noexcept(*get()))
      -> std::add_lvalue_reference_t<T> {
    return *get();
  }

  CXX23_CONSTEXPR pointer operator->() const noexcept {
    return get();
  }
};

template <typename T, typename... Args>
  requires(!std::is_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T> make_unique(Args &&...args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
  requires(!std::is_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T> make_unique_for_overwrite() {
  return unique_ptr<T>(new T);
}

template <typename T1, typename D1, typename T2, typename D2>
inline CXX23_CONSTEXPR bool operator==(const unique_ptr<T1, D1> &lhs,
                                       const unique_ptr<T2, D2> &rhs) {
  return lhs.get() == rhs.get();
}

template <typename T1, typename D1, typename T2, typename D2>
inline CXX23_CONSTEXPR bool operator<(const unique_ptr<T1, D1> &lhs,
                                      const unique_ptr<T2, D2> &rhs) {
  using CT = std::common_type_t<typename unique_ptr<T1, D1>::pointer,
                                typename unique_ptr<T2, D2>::pointer>;
  return std::less<CT>(lhs.get(), rhs.get());
}

template <typename T1, typename D1, typename T2, typename D2>
inline CXX23_CONSTEXPR bool operator<=(const unique_ptr<T1, D1> &lhs,
                                       const unique_ptr<T2, D2> &rhs) {
  return !(rhs < lhs);
}

template <typename T1, typename D1, typename T2, typename D2>
inline CXX23_CONSTEXPR bool operator>(const unique_ptr<T1, D1> &lhs,
                                      const unique_ptr<T2, D2> &rhs) {
  return rhs < lhs;
}

template <typename T1, typename D1, typename T2, typename D2>
inline CXX23_CONSTEXPR bool operator>=(const unique_ptr<T1, D1> &lhs,
                                       const unique_ptr<T2, D2> &rhs) {
  return !(lhs < rhs);
}

template <typename T1, typename D1, typename T2, typename D2>
  requires std::three_way_comparable_with<typename unique_ptr<T1, D1>::pointer,
                                          typename unique_ptr<T2, D2>::pointer>
inline auto operator<=>(const unique_ptr<T1, D1> &lhs,
                        const unique_ptr<T2, D2> &rhs)
    -> std::compare_three_way_result_t<typename unique_ptr<T1, D1>::pointer,
                                       typename unique_ptr<T2, D2>::pointer> {
  return std::compare_three_way{}(lhs.get(), rhs.get());
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator==(const unique_ptr<T, D> &x,
                                       std::nullptr_t) noexcept {
  return !x;
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator==(std::nullptr_t,
                                       const unique_ptr<T, D> &x) noexcept {
  return !x;
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator<(const unique_ptr<T, D> &x,
                                      std::nullptr_t) {
  return std::less<typename unique_ptr<T, D>::pointer>{}(x.get(), nullptr);
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator<(std::nullptr_t,
                                      const unique_ptr<T, D> &x) {
  return std::less<typename unique_ptr<T, D>::pointer>{}(nullptr, x.get());
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator<=(const unique_ptr<T, D> &x,
                                       std::nullptr_t) {
  return !(nullptr < x);
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator<=(std::nullptr_t,
                                       const unique_ptr<T, D> &x) {
  return !(x < nullptr);
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator>(const unique_ptr<T, D> &x,
                                      std::nullptr_t) {
  return nullptr < x;
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator>(std::nullptr_t,
                                      const unique_ptr<T, D> &x) {
  return x < nullptr;
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator>=(const unique_ptr<T, D> &x,
                                       std::nullptr_t) {
  return !(x < nullptr);
}

template <typename T, typename D>
inline CXX23_CONSTEXPR bool operator>=(std::nullptr_t,
                                       const unique_ptr<T, D> &x) {
  return !(nullptr < x);
}

template <typename T, typename D>
  requires std::three_way_comparable<typename unique_ptr<T, D>::pointer>
inline CXX23_CONSTEXPR auto operator<=>(const unique_ptr<T, D> &x,
                                        std::nullptr_t)
    -> std::compare_three_way_result_t<typename unique_ptr<T, D>::pointer> {
  return std::compare_three_way{}(
      x.get(), static_cast<typename unique_ptr<T, D>::pointer>(nullptr));
}

template <typename CharT, typename Traits, typename Y, typename D>
  requires requires(std::basic_ostream<CharT, Traits> &os,
                    const unique_ptr<Y, D> &p) { os << p.get(); }
inline std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, const unique_ptr<Y, D> &p) {
  return os << p.get();
}

} // namespace gkxx

namespace std {

template <typename T, typename D>
  requires std::is_swappable_v<D>
CXX23_CONSTEXPR void swap(gkxx::unique_ptr<T, D> &lhs,
                          gkxx::unique_ptr<T, D> &rhs) noexcept {
  lhs.swap(rhs);
}

template <typename T, typename D>
struct hash<gkxx::unique_ptr<T, D>> {
 private:
  using pointer_hash = std::hash<typename gkxx::unique_ptr<T, D>::pointer>;

 public:
  std::size_t operator()(const gkxx::unique_ptr<T, D> &p) const
    requires requires {
      { pointer_hash{}(p.get()) } -> std::same_as<std::size_t>;
    }
  {
    return pointer_hash{}(p.get());
  }
};

} // namespace std

#undef CXX23_CONSTEXPR

#endif // GKXX_UNIQUE_PTR_HPP