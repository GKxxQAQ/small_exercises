#ifndef GKXX_FUNCTION_HPP
#define GKXX_FUNCTION_HPP

#include <concepts>
#include <exception>
#include <memory>

#include "invoke.hpp"
#include "memory/unique_ptr.hpp"

namespace gkxx {

struct bad_function_call : public std::exception {
  const char *what() const noexcept override {
    return "bad function call";
  }
};

template <typename>
class function;

namespace detail {

  template <typename Signature>
  inline constexpr bool
  not_empty_function(function<Signature> const &f) noexcept {
    return f != nullptr;
  }
  inline constexpr bool not_empty_function(auto *fp) noexcept {
    return fp != nullptr;
  }
  template <typename T, typename Class>
  inline constexpr bool not_empty_function(T Class::*mp) noexcept {
    return mp != nullptr;
  }
  inline constexpr bool not_empty_function(auto &&) noexcept {
    return true;
  }

} // namespace detail

// TODO: small object optimization
template <typename R, typename... Args>
class function<R(Args...)> {
 public:
  using result_type = R;

  // (1)
  function() noexcept : pimpl{} {}

  // (2)
  function(std::nullptr_t) noexcept : pimpl{} {}

  // (3)
  function(const function &other) : pimpl{} {
    if (other.pimpl)
      pimpl = other.pimpl->clone();
  }

  // (4)
  function(function &&) noexcept = default;

  // (5)
  template <typename Fn>
    requires(std::is_invocable_r_v<R, std::decay_t<Fn> &, Args...> &&
             !std::is_same_v<std::decay_t<Fn>, function>)
  function(Fn &&fn) : pimpl{} { // TODO: noexcept-ness
    using target = std::decay_t<Fn>;

    static_assert(std::is_copy_constructible_v<target>,
                  "function target type must be copy constructible");
    static_assert(std::is_constructible_v<target, Fn>,
                  "function target type must be constructible from the "
                  "constructor argument");

    if (detail::not_empty_function(fn))
      pimpl = make_unique<wrapper<target>>(std::forward<Fn>(fn));
  }

  // (1), (2)
  function &operator=(function other) noexcept {
    swap(other);
    return *this;
  }

  // (3)
  function &operator=(std::nullptr_t) noexcept {
    pimpl.reset();
    return *this;
  }

  // (4)
  template <typename Fn>
    requires std::is_invocable_r_v<R, std::decay_t<Fn> &, Args...>
  function &operator=(Fn &&fn) {
    function(std::forward<Fn>(fn)).swap(*this);
    return *this;
  }

  // (5)
  template <typename F>
  function &operator=(std::reference_wrapper<F> fr) noexcept {
    function(fr).swap(*this);
    return *this;
  }

  ~function() = default;

  void swap(function &other) noexcept {
    pimpl.swap(other.pimpl);
  }

  explicit operator bool() const noexcept {
    return static_cast<bool>(pimpl);
  }

  R operator()(Args... args) const {
    if (!pimpl)
      throw bad_function_call{};
    return pimpl->invoke(std::forward<Args>(args)...);
  }

  const std::type_info &target_type() const noexcept {
    if (pimpl)
      return pimpl->get_typeid();
    else
      return typeid(void);
  }

  template <typename T>
  const T *target() const noexcept {
    if (target_type() == typeid(T))
      return static_cast<const T *>(pimpl->get_ptr());
    else
      return nullptr;
  }

  template <typename T>
  T *target() noexcept {
    return const_cast<T *>(static_cast<const function *>(this)->target());
  }

 private:
  struct wrapper_base {
    virtual ~wrapper_base() = default;
    virtual auto clone() const -> unique_ptr<wrapper_base> = 0;
    virtual auto invoke(Args... args) const -> R = 0;
    virtual auto get_typeid() const -> const std::type_info & = 0;
    virtual auto get_ptr() -> void * = 0;
  };
  template <typename F>
    requires std::same_as<F, std::decay_t<F>>
  struct wrapper final : public wrapper_base {
    F func;
    template <typename Fn>
    wrapper(Fn &&fn) : func(std::forward<Fn>(fn)) {}
    unique_ptr<wrapper_base> clone() const final {
      return make_unique<wrapper<F>>(func);
    }
    R invoke(Args... args) const final {
      return ::gkxx::invoke(func, std::forward<Args>(args)...);
    }
    const std::type_info &get_typeid() const final {
      return typeid(F);
    }
    void *get_ptr() final {
      return static_cast<void *>(&func);
    }
    ~wrapper() = default;
  };
  unique_ptr<wrapper_base> pimpl;
};

template <typename R, typename... Args>
function(R (*)(Args...)) -> function<R(Args...)>;

namespace detail {

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) noexcept(NE)) -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) & noexcept(NE)) -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) volatile noexcept(NE))
      -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) volatile & noexcept(NE))
      -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) const noexcept(NE))
      -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) const & noexcept(NE))
      -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) const volatile noexcept(NE))
      -> R (*)(A...);

  template <typename R, typename Class, typename... A, bool NE>
  auto deduce_signature_impl(R (Class::*)(A...) const volatile & noexcept(NE))
      -> R (*)(A...);

#if __cplusplus > 202002L
  template <typename R, typename... A, bool NE>
  auto deduce_signature_impl(R (*)(A...) noexcept(NE)) -> R (*)(A...);
#endif // C++23

  template <typename F>
  auto deduce_signature() noexcept {
    if constexpr (requires { &F::operator(); })
      return deduce_signature_impl(&F::operator());
    else
      return static_cast<void *>(nullptr);
  }

} // namespace detail

template <typename F>
  requires requires { &F::operator(); }
function(F)
    -> function<std::remove_pointer_t<decltype(detail::deduce_signature<F>())>>;

template <typename R, typename... Args>
inline bool operator==(const function<R(Args...)> &f, std::nullptr_t) noexcept {
  return !f;
}

template <typename R, typename... Args>
inline bool operator==(std::nullptr_t, const function<R(Args...)> &f) noexcept {
  return !f;
}

} // namespace gkxx

namespace std {

template <typename R, typename... Args>
void swap(gkxx::function<R(Args...)> &lhs,
          gkxx::function<R(Args...)> &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace std

#endif // GKXX_FUNCTION_HPP