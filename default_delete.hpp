#ifndef GKXX_DEFAULT_DELETE_HPP
#define GKXX_DEFAULT_DELETE_HPP

#include <concepts>

#ifndef CXX23_CONSTEXPR
#if __cplusplus > 202002L
#define CXX23_CONSTEXPR constexpr
#else
#define CXX23_CONSTEXPR
#endif
#endif

namespace gkxx {

namespace detail::default_delete {

  template <typename T>
  concept complete_type = !std::is_void_v<T> && requires { sizeof(T); };

} // namespace detail::default_delete

template <typename T>
struct default_delete {
  constexpr default_delete() noexcept = default;
  template <typename U>
    requires std::convertible<U *, T *>
  CXX23_CONSTEXPR default_delete(const default_delete<U> &) noexcept {}
  CXX23_CONSTEXPR void operator()(T *ptr) const noexcept(noexcept(delete ptr)) {
    static_assert(detail::default_delete::complete_type<T>,
                  "delete a pointer to incomplete type");
    delete ptr;
  }
};

template <typename T>
struct default_delete<T[]> {
  constexpr default_delete() noexcept = default;
  template <typename U>
    requires std::convertible<U (*)[], T (*)[]>
  CXX23_CONSTEXPR default_delete(const default_delete<U[]> &) noexcept {}
  template <typename U>
    requires std::convertible<U (*)[], T (*)[]>
  CXX23_CONSTEXPR void operator()(U *ptr) const
      noexcept(noexcept(delete[] ptr)) {
    static_assert(detail::default_delete::complete_type<T>,
                  "delete a pointer to incomplete type");
    delete[] ptr;
  }
};

} // namespace gkxx

#undef CXX23_CONSTEXPR

#endif // GKXX_DEFAULT_DELETE_HPP