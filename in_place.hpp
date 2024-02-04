#ifndef GKXX_IN_PLACE_HPP
#define GKXX_IN_PLACE_HPP

#include <cstddef>

namespace gkxx {

struct in_place_t {
  explicit in_place_t() = default;
};

inline constexpr in_place_t in_place{};

template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

template <std::size_t I>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template <std::size_t I>
inline constexpr in_place_index_t<I> in_place_index;

}

#endif // GKXX_IN_PLACE_HPP