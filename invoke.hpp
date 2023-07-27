#ifndef GKXX_INVOKE_HPP
#define GKXX_INVOKE_HPP

#include <type_traits>
#include <utility>

#include "is_specialization_of.hpp"

namespace gkxx {

namespace detail {

  template <typename MemType, typename Class, typename First, typename... Rest>
  inline constexpr decltype(auto) invoke_impl(MemType Class::*member,
                                              First &&first, Rest &&...rest)
    requires(std::is_function_v<MemType> || sizeof...(Rest) == 0)
  {
    using decayed_first = std::remove_cvref_t<First>;
    if constexpr (std::is_function_v<MemType>) {
      if constexpr (std::is_base_of_v<Class, decayed_first>)
        return (std::forward<First>(first).*
                member)(std::forward<Rest>(rest)...);
      else if constexpr (meta::is_specialization_of_v<decayed_first,
                                                      std::reference_wrapper>)
        return (std::forward<First>(first).get().*
                member)(std::forward<Rest>(rest)...);
      else
        return ((*std::forward<First>(first)).*
                member)(std::forward<Rest>(rest)...);
    } else {
      if constexpr (std::is_base_of_v<Class, decayed_first>)
        return std::forward<First>(first).*member;
      else if constexpr (meta::is_specialization_of_v<decayed_first,
                                                      std::reference_wrapper>)
        return std::forward<First>(first).get().*member;
      else
        return (*std::forward<First>(first)).*member;
    }
  }

  template <typename F, typename... Ts>
  inline constexpr decltype(auto) invoke_impl(F &&func, Ts &&...params) {
    return std::forward<F>(func)(std::forward<Ts>(params)...);
  }

} // namespace detail

template <typename F, typename... Ts>
inline constexpr decltype(auto) invoke(F &&func, Ts &&...params) {
  return detail::invoke_impl(std::forward<F>(func),
                             std::forward<Ts>(params)...);
}

template <typename R, typename F, typename... Ts>
inline constexpr R invoke_r(F &&func, Ts &&...params) {
  if constexpr (std::is_void_v<R>)
    invoke(std::forward<F>(func), std::forward<Ts>(params)...);
  else
    return invoke(std::forward<F>(func), std::forward<Ts>(params)...);
}

} // namespace gkxx

#endif // GKXX_INVOKE_HPP