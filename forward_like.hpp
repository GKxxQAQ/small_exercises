#ifndef GKXX_FORWARD_LIKE_HPP
#define GKXX_FORWARD_LIKE_HPP

#include <type_traits>

namespace gkxx::meta {

// C++23 std::forward_like
template <typename T, typename U>
inline constexpr auto &&forward_like(U &&x) noexcept {
  using T_unref = std::remove_reference_t<T>;
  using U_unref = std::remove_reference_t<U>;
  using referenced_type =
      std::conditional_t<std::is_const_v<T_unref>, std::add_const_t<U_unref>,
                         U_unref>;
  if constexpr (std::is_lvalue_reference_v<T &&>)
    return static_cast<std::add_lvalue_reference_t<referenced_type>>(x);
  else
    return static_cast<std::add_rvalue_reference_t<referenced_type>>(x);
}

} // namespace gkxx::meta

#endif // GKXX_FORWARD_LIKE_HPP