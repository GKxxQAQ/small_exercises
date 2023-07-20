#include "../../generator.hpp"
#include <chrono>
#include <coroutine>
#include <iostream>
#include <memory>
#include <utility>
#include <optional>

gkxx::Generator<unsigned> range(unsigned n) {
  unsigned i{};
  while (i < n)
    co_yield i++;
}

constexpr unsigned N = 300000000u;

template <typename Func, typename... Args>
auto timer(Func &&func, Args &&...args) {
  using clock_type = std::chrono::steady_clock;
  auto start = clock_type::now();
  std::forward<Func>(func)(std::forward<Args>(args)...);
  auto end = clock_type::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

auto test_range() {
  unsigned result{};
  for (auto i : range(N))
    result ^= i * i;
  return result;
}

auto test_loop() {
  unsigned result{};
  for (unsigned i{}; i != N; ++i)
    result ^= i * i;
  return result;
}

auto test_unique_ptr() {
  unsigned result{};
  for (auto i{std::make_unique<unsigned>()}; *i != N; i = std::make_unique<unsigned>(*i + 1))
    result ^= *i * *i;
  return result;
}

auto test_new() {
  unsigned result{}, tmp;
  for (auto i{new unsigned{}}; *i != N; tmp = *i, delete i, i = new unsigned{tmp + 1})
    result ^= *i * *i;
  return result;
}

auto test_optional() {
  unsigned result{};
  for (auto i{std::make_optional<unsigned>()}; *i != N; i.emplace(*i + 1))
    result ^= *i * *i;
  return result;
}

auto main() -> int {
  std::cout << "test_loop: " << timer(test_loop) << '\n';
  std::cout << "test_unique_ptr: " << timer(test_unique_ptr) << '\n';
  std::cout << "test_new: " << timer(test_new) << '\n';
  std::cout << "test_range: " << timer(test_range) << '\n';
  std::cout << "test_optional: " << timer(test_optional) << '\n';
  return 0;
}