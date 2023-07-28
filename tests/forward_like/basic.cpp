#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "../../forward_like.hpp"

struct TypeTeller {
  void operator()() & {
    std::cout << "mutable lvalue\n";
  }
  void operator()() const & {
    std::cout << "const lvalue\n";
  }
  void operator()() && {
    std::cout << "mutable rvalue\n";
  }
  void operator()() const && {
    std::cout << "const rvalue\n";
  }
};

struct FarStates {
  std::unique_ptr<TypeTeller> ptr;
  std::optional<TypeTeller> opt;
  std::vector<TypeTeller> container;
};

auto &&from_opt(auto &&fs) {
  return gkxx::meta::forward_like<decltype(fs)>(fs.opt.value());
}
auto &&subscript(auto &&fs, std::size_t i) {
  return gkxx::meta::forward_like<decltype(fs)>(fs.container.at(i));
}
auto &&from_ptr(auto &&fs) {
  if (!fs.ptr)
    throw std::bad_optional_access{};
  return gkxx::meta::forward_like<decltype(fs)>(*fs.ptr);
}

int main() {
  FarStates my_state{
      .ptr{std::make_unique<TypeTeller>()},
      .opt = std::optional<TypeTeller>{std::in_place, TypeTeller{}},
      .container{std::vector<TypeTeller>(1)},
  };

  from_ptr(my_state)();
  from_opt(my_state)();
  subscript(my_state, 0)();

  std::cout << '\n';

  from_ptr(std::as_const(my_state))();
  from_opt(std::as_const(my_state))();
  subscript(std::as_const(my_state), 0)();

  std::cout << '\n';

  from_ptr(std::move(my_state))();
  from_opt(std::move(my_state))();
  subscript(std::move(my_state), 0)();

  std::cout << '\n';

  from_ptr(std::move(std::as_const(my_state)))();
  from_opt(std::move(std::as_const(my_state)))();
  subscript(std::move(std::as_const(my_state)), 0)();

  std::cout << '\n';
}