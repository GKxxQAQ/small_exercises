#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};

class any {
  struct wrapper_base {
    virtual ~wrapper_base() = default;
    virtual std::unique_ptr<wrapper_base> clone() const = 0;
    virtual const std::type_info &get_typeid() const = 0;
  };
  template <typename ValueType>
  struct wrapper final : public wrapper_base {
    static_cast(std::is_same_v<ValueType, std::decay_t<ValueType>>);

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

  any(const any &other) : pimpl(other.pimpl->clone()) {}
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
  any(in_place_type_t<ValueType>, Args &&...args)
      : pimpl(std::make_unique<wrapper<std::decay_t<ValueType>>>(
            std::forward<Args>(args)...)) {}

  template <typename ValueType, typename... Args>
    requires(std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
             std::is_copy_constructible_v<std::decay_t<ValueType>>)
  std::decay_t<ValueType> &emplace(Args &&...args) {
    pimpl = std::make_unique<wrapper<std::decay_t<ValueType>>>(
        std::forward<Args>(args)...);
    return *pimpl;
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
    return &static_cast<wrapper<T> *>(pimpl.get()).thing;
  }
  template <typename T>
  T *get_raw_ptr() noexcept {
    return &static_cast<wrapper<T> *>(pimpl.get()).thing;
  }

  template <typename T>
  friend const T *any_cast(const any *operand) noexcept {
    if (operand && typeid(T) == operand->type())
      return operand->get_raw_ptr<T>();
    return nullptr;
  }

  template <typename T>
  friend T *any_cast(any *operand) noexcept {
    if (operand && typeid(T) == operand->type())
      return operand->get_raw_ptr<T>();
    return nullptr;
  }
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