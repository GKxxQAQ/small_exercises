#ifndef GKXX_ANY_HPP
#define GKXX_ANY_HPP

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <algorithm>

namespace gkxx {

template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

struct bad_any_cast : public std::bad_cast {
  const char *what() const noexcept override {
    return "bad any_cast";
  }
};

class any {
  struct wrapper_base {
    virtual ~wrapper_base() = default;
    virtual std::unique_ptr<wrapper_base> clone() const = 0;
    virtual const std::type_info &get_typeid() const = 0;
  };

  template <typename ValueType>
  struct wrapper final : public wrapper_base {
    static_assert(std::is_same_v<ValueType, std::decay_t<ValueType>>);

    ValueType thing;

    template <typename... Args>
    wrapper(Args &&...args) : thing(std::forward<Args>(args)...) {}
    std::unique_ptr<wrapper_base> clone() const final {
      return std::make_unique<wrapper<ValueType>>(thing);
    }
    const std::type_info &get_typeid() const final {
      return typeid(ValueType);
    }
  };
  std::unique_ptr<wrapper_base> pimpl{};

  template <typename>
  struct is_in_place_type : public std::false_type {};
  template <typename T>
  struct is_in_place_type<in_place_type_t<T>> : public std::true_type {};

 public:
  constexpr any() noexcept = default;

  any(const any &other) : pimpl(other.pimpl ? other.pimpl->clone() : nullptr) {}
  any(any &&) noexcept = default;
  void swap(any &other) noexcept {
    pimpl.swap(other.pimpl);
  }
  any &operator=(any other) noexcept {
    swap(other);
    return *this;
  }
  ~any() = default;

  template <typename ValueType>
    requires(!std::is_same_v<std::decay_t<ValueType>, any> &&
             !is_in_place_type<std::decay_t<ValueType>>::value &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  any(ValueType &&x)
      : pimpl(std::make_unique<wrapper<std::decay_t<ValueType>>>(
            std::forward<ValueType>(x))) {}

  template <typename ValueType, typename... Args>
    requires(std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  explicit any(in_place_type_t<ValueType>, Args &&...args)
      : pimpl(std::make_unique<wrapper<std::decay_t<ValueType>>>(
            std::forward<Args>(args)...)) {}

  template <typename ValueType, typename U, typename... Args>
    requires(std::is_constructible_v<std::decay_t<ValueType>,
                                     std::initializer_list<U> &, Args...> &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  explicit any(in_place_type_t<ValueType>, std::initializer_list<U> il,
               Args &&...args)
      : pimpl(std::make_unique<wrapper<std::decay_t<ValueType>>>(
            il, std::forward<Args>(args)...)) {}

  template <typename ValueType, typename... Args>
    requires(std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  std::decay_t<ValueType> &emplace(Args &&...args) {
    auto ptr = std::make_unique<wrapper<std::decay_t<ValueType>>>(
        std::forward<Args>(args)...);
    auto &ret = ptr->thing;
    pimpl = std::move(ptr);
    return ret;
  }

  template <typename ValueType, typename U, typename... Args>
    requires(std::is_constructible_v<std::decay_t<ValueType>,
                                     std::initializer_list<U> &, Args...> &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  std::decay_t<ValueType> &emplace(std::initializer_list<U> il,
                                   Args &&...args) {
    auto ptr = std::make_unique<wrapper<std::decay_t<ValueType>>>(
        il, std::forward<Args>(args)...);
    auto &ret = ptr->thing;
    pimpl = std::move(ptr);
    return ret;
  }

  void reset() noexcept {
    pimpl.reset();
  }

  bool has_value() const noexcept {
    return !!pimpl;
  }

  const std::type_info &type() const noexcept {
    if (pimpl)
      return pimpl->get_typeid();
    else
      return typeid(void);
  }

 private:
  template <typename T>
  const T *get_raw_ptr() const noexcept {
    return &static_cast<wrapper<T> *>(pimpl.get())->thing;
  }
  template <typename T>
  T *get_raw_ptr() noexcept {
    return &static_cast<wrapper<T> *>(pimpl.get())->thing;
  }

  template <typename T>
  friend const T *any_cast(const any *operand) noexcept;

  template <typename T>
  friend T *any_cast(any *operand) noexcept;
};

template <typename T>
inline T any_cast(const any &operand) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;
  static_assert(std::is_constructible_v<T, const U &>);
  auto ptr = any_cast<U>(&operand);
  if (!ptr)
    throw bad_any_cast{};
  return static_cast<T>(*ptr);
}

template <typename T>
inline T any_cast(any &operand) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;
  static_assert(std::is_constructible_v<T, U &>);
  auto ptr = any_cast<U>(&operand);
  if (!ptr)
    throw bad_any_cast{};
  return static_cast<T>(*ptr);
}

template <typename T>
inline T any_cast(any &&operand) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;
  static_assert(std::is_constructible_v<T, U>);
  auto ptr = any_cast<U>(&operand);
  if (!ptr)
    throw bad_any_cast{};
  return static_cast<T>(std::move(*ptr));
}

template <typename T>
inline const T *any_cast(const any *operand) noexcept {
  if (operand && typeid(T) == operand->type())
    return operand->get_raw_ptr<T>();
  return nullptr;
}

template <typename T>
inline T *any_cast(any *operand) noexcept {
  if (operand && typeid(T) == operand->type())
    return operand->get_raw_ptr<T>();
  return nullptr;
}

template <typename T, typename... Args>
inline any make_any(Args &&...args) {
  return any(in_place_type<T>, std::forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
inline any make_any(std::initializer_list<U> il, Args &&...args) {
  return any(in_place_type<T>, il, std::forward<Args>(args)...);
}

} // namespace gkxx

namespace std {

template <>
inline void swap(gkxx::any &lhs, gkxx::any &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace std

#endif // GKXX_ANY_HPP