#include <memory>
#include <type_traits>
#include <utility>
#include <typeinfo>

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
      : pimpl(std::make_unique<std::decay_t<ValueType>>(
            std::forward<ValueType>(x))) {}

  template <typename ValueType, typename... Args>
  requires(std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
           std::is_copy_constructible_v<std::decay_t<ValueType>>)
  any(in_place_type_t<ValueType>, Args &&...args)
      : pimpl(std::make_unique<std::decay_t<ValueType>>(
            std::forward<Args>(args)...)) {}

  template <typename ValueType, typename... Args>
  requires(std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
           std::is_copy_constructible_v<std::decay_t<ValueType>>)
  std::decay_t<ValueType> &emplace(Args &&...args) {
    pimpl = std::make_unique<std::decay_t<ValueType>>(std::forward<Args>(args)...);
    return *pimpl;
  }

  void reset() noexcept {
    pimpl.reset();
  }

  bool has_value() const noexcept {
    return static_cast<bool>(pimpl);
  }

  const std::type_info &type() const noexcept {
    if (pimpl)
      return pimpl->get_typeid();
    else
      return typeid(void);
  }
};