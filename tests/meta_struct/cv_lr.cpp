#include "../../meta_struct.hpp"

#include <concepts>
#include <iostream>
#include <string>
#include <utility>

int main() {
  using namespace gkxx::meta_struct;
  using Person = meta_struct<
      member<"id", int>,
      member<"score", int, [](auto &self) { return get<"id">(self) + 1; }>,
      member<"name", std::string, [] { return "John"; }>>;
  Person p;
  static_assert(std::same_as<decltype(get<"name">(p)), std::string &>);
  static_assert(
      std::same_as<decltype(get<"name">(std::move(p))), std::string &&>);
  static_assert(std::same_as<decltype(get<"name">(std::as_const(p))),
                             const std::string &>);
  static_assert(std::same_as<decltype(get<"name">(std::move(std::as_const(p)))),
                             const std::string &&>);
}