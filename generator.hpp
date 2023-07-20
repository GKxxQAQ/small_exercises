#ifndef GKXX_EXERCISE_GENERATOR_HPP
#define GKXX_EXERCISE_GENERATOR_HPP

#include <concepts>
#include <coroutine>
#include <exception>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>

namespace gkxx {

template <typename Yielded>
class Generator {
 public:
  class promise_type;

 private:
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type m_coro_handle;

  explicit Generator(promise_type &p)
      : m_coro_handle{handle_type::from_promise(p)} {}

 public:
  Generator(const Generator &) = delete;
  Generator(Generator &&other) noexcept
      : m_coro_handle{std::exchange(other.m_coro_handle, nullptr)} {}
  void swap(Generator &other) noexcept {
    std::swap(m_coro_handle, other.m_coro_handle);
  }
  Generator &operator=(Generator other) noexcept {
    other.swap(*this);
    return *this;
  }
  ~Generator() {
    if (m_coro_handle)
      m_coro_handle.destroy();
  }

 private:
  // iterator
  class iterator;

 public:
  iterator begin() noexcept {
    iterator ret{m_coro_handle};
    if (!m_coro_handle.promise().has_value())
      ++ret;
    return ret;
  }
  std::default_sentinel_t end() const noexcept {
    return {};
  }
};

template <typename Yielded>
class Generator<Yielded>::promise_type {
 private:
  std::optional<Yielded> m_value{};
  std::exception_ptr m_exception;

 public:
  Generator<Yielded> get_return_object() noexcept {
    return Generator{*this};
  }

  std::suspend_always initial_suspend() const noexcept {
    return {};
  }

  std::suspend_always final_suspend() const noexcept {
    return {};
  }

  void unhandled_exception() noexcept(
      std::is_nothrow_copy_assignable_v<std::exception_ptr>) {
    m_exception = std::current_exception();
  }

  template <std::convertible_to<Yielded> Type = Yielded>
  std::suspend_always yield_value(Type &&val) {
    m_value.emplace(std::forward<Type>(val));
    return {};
  }

  void return_void() const noexcept {}

  [[nodiscard]] Yielded const &get_value() const noexcept {
    return *m_value;
  }

  [[nodiscard]] bool has_value() const noexcept {
    return static_cast<bool>(m_value);
  }
};

template <typename Yielded>
class Generator<Yielded>::iterator {
 private:
  handle_type m_coro_handle;

 public:
  using value_type = Yielded;
  using reference = Yielded const &;
  using pointer = Yielded const *;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;

  iterator(const handle_type &handle) : m_coro_handle{handle} {}

  iterator(const iterator &) = delete;
  iterator(iterator &&other) noexcept
      : m_coro_handle{std::exchange(other.m_coro_handle, nullptr)} {}
  void swap(iterator &other) noexcept {
    std::swap(m_coro_handle, other.m_coro_handle);
  }
  iterator &operator=(iterator other) noexcept {
    other.swap(*this);
    return *this;
  }

  bool operator==(std::default_sentinel_t) const noexcept {
    return m_coro_handle.done();
  }

  iterator &operator++() {
    m_coro_handle.resume();
    return *this;
  }

  void operator++(int) {
    ++*this;
  }

  reference operator*() noexcept {
    return m_coro_handle.promise().get_value();
  }
};

} // namespace gkxx

#endif // GKXX_EXERCISE_GENERATOR_HPP