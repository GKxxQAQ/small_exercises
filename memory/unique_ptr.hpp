#ifndef GKXX_UNIQUE_PTR_HPP
#define GKXX_UNIQUE_PTR_HPP

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

#ifndef NO_UNIQUE_ADDRESS
#ifdef _MSC_VER
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

namespace gkxx {

namespace detail {

  template <typename T>
  concept complete_type = !std::is_void_v<T> && requires { sizeof(T); };

} // namespace detail

template <typename T>
struct default_delete {
  constexpr default_delete() noexcept = default;
  template <typename U>
    requires std::convertible_to<U *, T *>
  CXX23_CONSTEXPR default_delete(const default_delete<U> &) noexcept {}
  CXX23_CONSTEXPR void operator()(T *ptr) const noexcept(noexcept(delete ptr)) {
    static_assert(detail::complete_type<T>,
                  "delete a pointer to incomplete type");
    delete ptr;
  }
};

template <typename T>
struct default_delete<T[]> {
  constexpr default_delete() noexcept = default;
  template <typename U>
    requires std::convertible_to<U (*)[], T (*)[]>
  CXX23_CONSTEXPR default_delete(const default_delete<U[]> &) noexcept {}
  template <typename U>
    requires std::convertible_to<U (*)[], T (*)[]>
  CXX23_CONSTEXPR void operator()(U *ptr) const
      noexcept(noexcept(delete[] ptr)) {
    static_assert(detail::complete_type<T>,
                  "delete a pointer to incomplete type");
    delete[] ptr;
  }
};

namespace detail {

  template <typename D>
  struct unique_ptr_deleter_traits {
    static_assert(!std::is_rvalue_reference_v<D>,
                  "unique_ptr's deleter type must be a function object type"
                  " or an lvalue reference type");

    static inline constexpr auto has_default =
        std::is_default_constructible_v<D> && !std::is_pointer_v<D>;

    static inline constexpr auto is_ref = std::is_reference_v<D>;

    template <typename E>
    static inline constexpr auto compatible =
        (is_ref && std::same_as<D, E>) ||
        (!is_ref && std::convertible_to<E, D>);
  };

  template <typename T, typename D>
  struct unique_ptr_pointer_traits {
    using pointer = decltype([] {
      if constexpr (requires { typename std::remove_reference_t<D>::pointer; })
        return typename std::remove_reference_t<D>::pointer{};
      else
        return static_cast<T *>(nullptr);
    }());
  };

} // namespace detail

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
  using dtr = detail::unique_ptr_deleter_traits<Deleter>;
  using ptr = detail::unique_ptr_pointer_traits<T, Deleter>;

 public:
  using pointer = typename ptr::pointer;
  using element_type = T;
  using deleter_type = Deleter;

 private:
  pointer m_ptr;
  NO_UNIQUE_ADDRESS Deleter m_deleter;

 public:
  // (1)
  constexpr unique_ptr() noexcept
    requires(dtr::has_default)
      : m_ptr(), m_deleter() {}

  constexpr unique_ptr(std::nullptr_t) noexcept
    requires(dtr::has_default)
      : unique_ptr() {}

  // (2)
  CXX23_CONSTEXPR explicit unique_ptr(pointer p) noexcept
    requires(dtr::has_default)
      : m_ptr(p), m_deleter() {}

  // (3) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(pointer p, const Deleter &d) noexcept
    requires(!dtr::is_ref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(pointer p, Deleter &&d) noexcept
    requires(!dtr::is_ref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(std::move(d)) {}

  // (3) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(pointer p, Deleter d) noexcept
    requires(dtr::is_ref && std::is_constructible_v<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(pointer p, std::remove_reference_t<Deleter> &&d)
    requires(dtr::is_ref && std::is_constructible_v<Deleter, decltype(d)>)
  = delete;

  // (5) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(!dtr::is_ref && std::is_move_constructible_v<Deleter>)
      : m_ptr(other.release()), m_deleter(std::move(other.get_deleter())) {}

  // (5) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(dtr::is_ref)
      : m_ptr(other.release()), m_deleter(other.get_deleter()) {}

  // (6)
  template <typename U, typename E>
  CXX23_CONSTEXPR unique_ptr(unique_ptr<U, E> &&other) noexcept
    requires(std::convertible_to<typename unique_ptr<U, E>::pointer, pointer> &&
             !std::is_array_v<U> && dtr::template compatible<E>)
      : m_ptr(other.release()),
        m_deleter(std::forward<E>(other.get_deleter())) {}

  CXX23_CONSTEXPR ~unique_ptr() {
    if (get() != nullptr)
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
    if (auto old_ptr = std::exchange(m_ptr, p))
      get_deleter()(old_ptr);
  }

  void swap(unique_ptr &other) noexcept {
    using std::swap;
    swap(m_ptr, other.m_ptr);
    swap(m_deleter, other.m_deleter);
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

  unique_ptr(const unique_ptr &) = delete;
  unique_ptr &operator=(const unique_ptr &) = delete;
};

namespace detail {

  template <typename T, typename D>
  struct unique_ptr_pointer_traits<T[], D> {
    using pointer = decltype([] {
      if constexpr (requires { typename std::remove_reference_t<D>::pointer; })
        return typename std::remove_reference_t<D>::pointer{};
      else
        return static_cast<T *>(nullptr);
    }());

    static inline constexpr auto ptr_elem_consistent =
        std::same_as<pointer, T *>;

    template <typename RawPtr>
    static inline constexpr auto compatible =
        ptr_elem_consistent &&
        std::convertible_to<std::remove_pointer_t<RawPtr> (*)[], T (*)[]>;

    template <>
    static inline constexpr auto compatible<std::nullptr_t> = true;

    template <typename U, typename E>
    static inline constexpr auto compatible<unique_ptr<U, E>> =
        std::is_array_v<U> && ptr_elem_consistent &&
        std::same_as<typename unique_ptr<U, E>::pointer,
                     typename unique_ptr<U, E>::element_type *> &&
        std::convertible_to<typename unique_ptr<U, E>::element_type (*)[],
                            T (*)[]>;
  };
} // namespace detail

template <typename T, typename Deleter>
class unique_ptr<T[], Deleter> {
  using dtr = detail::unique_ptr_deleter_traits<Deleter>;
  using ptr = detail::unique_ptr_pointer_traits<T[], Deleter>;

 public:
  using pointer = typename ptr::pointer;
  using element_type = T;
  using deleter_type = Deleter;

 private:
  pointer m_ptr;
  NO_UNIQUE_ADDRESS Deleter m_deleter;

 public:
  // (1)
  constexpr unique_ptr() noexcept
    requires(dtr::has_default)
      : m_ptr(), m_deleter() {}

  constexpr unique_ptr(std::nullptr_t) noexcept
    requires(dtr::has_default)
      : unique_ptr() {}

  // (2)
  template <typename Up>
  explicit unique_ptr(Up p) noexcept
    requires(dtr::has_default && ptr::template compatible<Up>)
      : m_ptr(p), m_deleter() {}

  // (3) for non-reference Deleter
  template <typename Up>
  CXX23_CONSTEXPR unique_ptr(Up p, const Deleter &d) noexcept
    requires(ptr::template compatible<Up> && !dtr::is_ref &&
             std::constructible_from<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for non-reference Deleter
  template <typename Up>
  CXX23_CONSTEXPR unique_ptr(Up p, Deleter &&d) noexcept
    requires(ptr::template compatible<Up> && !dtr::is_ref &&
             std::constructible_from<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(std::move(d)) {}

  // (3) for Deleter as lvalue reference
  template <typename Up>
  CXX23_CONSTEXPR unique_ptr(Up p, Deleter d) noexcept
    requires(ptr::template compatible<Up> && dtr::is_ref &&
             std::constructible_from<Deleter, decltype(d)>)
      : m_ptr(p), m_deleter(d) {}

  // (4) for Deleter as lvalue reference
  template <typename Up>
  CXX23_CONSTEXPR unique_ptr(Up p, std::remove_reference_t<Deleter> &&d)
    requires(ptr::template compatible<Up> && dtr::is_ref &&
             std::constructible_from<Deleter, decltype(d)>)
  = delete;

  // (5) for non-reference Deleter
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(!dtr::is_ref && std::is_move_constructible_v<Deleter>)
      : m_ptr(other.release()), m_deleter(std::move(other.get_deleter())) {}

  // (5) for Deleter as lvalue reference
  CXX23_CONSTEXPR unique_ptr(unique_ptr &&other) noexcept
    requires(dtr::is_ref)
      : m_ptr(other.release()), m_deleter(other.get_deleter()) {}

  // (6)
  template <typename U, typename E>
  CXX23_CONSTEXPR unique_ptr(unique_ptr<U, E> &&other) noexcept
    requires(ptr::template compatible<unique_ptr<U, E>> &&
             dtr::template compatible<E>)
      : m_ptr(other.release()),
        m_deleter(std::forward<E>(other.get_deleter())) {}

  CXX23_CONSTEXPR ~unique_ptr() {
    if (get() != nullptr)
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
    do_assign(std::move(other));
    return *this;
  }

  // (2) converting assignment operator
  template <typename U, typename E>
  CXX23_CONSTEXPR unique_ptr &operator=(unique_ptr &&other) noexcept
    requires(ptr::template compatible<unique_ptr<U, E>> &&
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

  template <typename Up>
  CXX23_CONSTEXPR void reset(Up p) noexcept
    requires(std::same_as<Up, pointer> || ptr::template compatible<Up>)
  {
    if (auto old_ptr = std::exchange(m_ptr, p))
      get_deleter()(old_ptr);
  }

  CXX23_CONSTEXPR void reset(std::nullptr_t = nullptr) noexcept {
    reset(pointer());
  }

  void swap(unique_ptr &other) noexcept {
    using std::swap;
    swap(m_ptr, other.m_ptr);
    swap(m_deleter, other.m_deleter);
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

  element_type &operator[](std::size_t i) const {
    return get()[i];
  }

  unique_ptr(const unique_ptr &) = delete;
  unique_ptr &operator=(const unique_ptr &) = delete;
};

template <typename T, typename... Args>
  requires(!std::is_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T> make_unique(Args &&...args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
  requires(std::is_unbounded_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T> make_unique(std::size_t size) {
  return unique_ptr<T>(new std::remove_extent_t<T>[size]());
}

template <typename T, typename... Args>
  requires(std::is_bounded_array_v<T>)
void make_unique(Args &&...) = delete;

template <typename T>
  requires(!std::is_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T> make_unique_for_overwrite() {
  return unique_ptr<T>(new T);
}

template <typename T>
  requires(std::is_unbounded_array_v<T>)
inline CXX23_CONSTEXPR unique_ptr<T>
make_unique_for_overwrite(std::size_t size) {
  return unique_ptr(new std::remove_extent_t<T>[size]);
}

template <typename T, typename... Args>
  requires(std::is_bounded_array_v<T>)
void make_unique_for_overwrite(Args &&...) = delete;

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

#undef NO_UNIQUE_ADDRESS
#undef CXX23_CONSTEXPR

#endif // GKXX_UNIQUE_PTR_HPP