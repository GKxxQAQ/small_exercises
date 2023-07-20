#ifndef GKXX_THROWING_HPP
#define GKXX_THROWING_HPP

#include <coroutine>
#include <utility>

namespace gkxx {

template <typename Type>
struct exit_condition;

template <typename Type>
struct [[nodiscard]] throwing {
  struct promise_type {
    throwing get_return_object() {
      return throwing{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_never initial_suspend();
    std::suspend_always final_suspend() noexcept;

    template <typename U>
    std::suspend_always yield_value(U &&value) {
      condition.store_exception(std::forward<U>(value));
      return {};
    }
    template <typename U>
    void return_value(U &&value) {
      condition.store_value(std::forward<U>(value));
    }
    void unhandled_exception() {}

    exit_condition<Type> condition;
  };

  throwing() = default;
  explicit throwing(std::coroutine_handle<promise_type> h) : handle{h} {}
  throwing(throwing &&other) noexcept
      : handle{std::exchange(other.handle, nullptr)} {}
  ~throwing() {
    if (handle)
      handle.destroy();
  }

  bool await_ready() {
    return handle.promise().condition.is_value();
  }
  Type await_resume() {
    return std::move(handle.promise().condition).value;
  }
  template <typename OuterPromise>
  void await_suspend(std::coroutine_handle<OuterPromise> outer_handle) {
    outer_handle.promise().condition.propogate_error(
        handle.promise().condition);
  }

  std::coroutine_handle<promise_type> handle{};
};

} // namespace gkxx

#endif // GKXX_THROWING_HPP