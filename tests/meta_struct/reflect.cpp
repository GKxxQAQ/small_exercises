#include "../../meta_struct.hpp"
#include "../../type_name.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main() {
  using namespace gkxx::meta_struct;
  using Person = meta_struct<
      member<"id", int>,
      member<"score", double, [](auto &self) { return get<"id">(self) + 1; }>,
      member<"name", std::string, [] { return "John"; }>>;
  auto vsv = Person::members::tags::as_container<std::vector>();
  for (auto sv : vsv)
    std::cout << sv << std::endl;
  Person::members::element_types::apply(
      []<typename T>(T *) { std::cout << gkxx::get_type_name<T>() << ' '; });
  std::cout << std::endl;
  Person::members::apply(
      []<typename T>(T *) { std::cout << gkxx::get_type_name<T>() << ' '; });
  static_assert(Person::members::contains_tag<"id">);
  static_assert(Person::members::contains_tag<"score">);
  static_assert(Person::members::contains_tag<"name">);
  static_assert(!Person::members::contains_tag<"school">);
  static_assert(Person::members::contains_element<int>);
  static_assert(Person::members::contains_element<double>);
  static_assert(Person::members::contains_element<std::string>);
  static_assert(!Person::members::contains_element<float>);
  static_assert(!Person::members::contains_element<std::string_view>);
  return 0;
}